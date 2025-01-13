#ifndef _PLYODINE_READERS_TRIANGLE_MESH_READER_
#define _PLYODINE_READERS_TRIANGLE_MESH_READER_

#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <expected>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_reader.h"

namespace plyodine {

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
class TriangleMeshReader : public PlyReader {
 protected:
  virtual void Start() = 0;

  virtual void AddVertex(const std::array<LocationType, 3> &position,
                         const std::array<NormalType, 3> *maybe_normal,
                         const std::array<UVType, 2> *maybe_uv) = 0;

  virtual void AddFace(const std::array<FaceIndexType, 3> &face) = 0;

 private:
  enum class ErrorCode : int {
    MIN_VALUE = 1,
    INVALID_VERTEX_TYPE = 1,
    INVALID_VERTEX_INDEX_TYPE = 2,
    INVALID_NORMAL_TYPE = 3,
    INVALID_TEXTURE_COORDINATE_TYPE = 4,
    MISSING_VERTEX_DIMENSION = 5,
    MISSING_VERTEX_INDICES = 6,
    VERTEX_X_NOT_FINITE = 7,
    VERTEX_Y_NOT_FINITE = 8,
    VERTEX_Z_NOT_FINITE = 9,
    NORMAL_X_NOT_FINITE = 10,
    NORMAL_Y_NOT_FINITE = 12,
    NORMAL_Z_NOT_FINITE = 13,
    TEXTURE_U_NOT_FINITE = 14,
    TEXTURE_V_NOT_FINITE = 15,
    VERTEX_INDEX_OUT_OF_RANGE = 16,
    MAX_VALUE = 16,
  };

  static class ErrorCategory final : public std::error_category {
    const char *name() const noexcept override {
      return "plyodine::TriangleMeshReader";
    }

    std::string message(int condition) const override {
      ErrorCode error_code{condition};
      switch (error_code) {
        case ErrorCode::INVALID_VERTEX_TYPE:
          return "The type of properties x, y, and z, on vertex elements must "
                 "be either float or double";
        case ErrorCode::INVALID_VERTEX_INDEX_TYPE:
          return "The type of property vertex_indices on face elements must be "
                 "an integral list type";
        case ErrorCode::INVALID_NORMAL_TYPE:
          return "The type of properties nx, ny, and nz, on vertex elements "
                 "must be either float or double";
        case ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE:
          return "The type of properties texture_s, texture_t, texture_u, "
                 "texture_v, s, t, u, and v on vertex elements must be either "
                 "float or double";
        case ErrorCode::MISSING_VERTEX_DIMENSION:
          return "Element vertex must have properties x, y, and z";
        case ErrorCode::MISSING_VERTEX_INDICES:
          return "Element face must have property vertex_indices";
        case ErrorCode::VERTEX_X_NOT_FINITE:
          return "Input contained a non-finite value for x";
        case ErrorCode::VERTEX_Y_NOT_FINITE:
          return "Input contained a non-finite value for y";
        case ErrorCode::VERTEX_Z_NOT_FINITE:
          return "Input contained a non-finite value for z";
        case ErrorCode::NORMAL_X_NOT_FINITE:
          return "Input contained a non-finite value for nx";
        case ErrorCode::NORMAL_Y_NOT_FINITE:
          return "Input contained a non-finite value for ny";
        case ErrorCode::NORMAL_Z_NOT_FINITE:
          return "Input contained a non-finite value for nz";
        case ErrorCode::TEXTURE_U_NOT_FINITE:
          return "Input contained a non-finite value for u";
        case ErrorCode::TEXTURE_V_NOT_FINITE:
          return "Input contained a non-finite value for v";
        case ErrorCode::VERTEX_INDEX_OUT_OF_RANGE:
          return "A vertex index was out of range";
      }

      return "Unknown Error";
    }

    std::error_condition default_error_condition(
        int value) const noexcept override {
      if (value < static_cast<int>(ErrorCode::MIN_VALUE) ||
          value > static_cast<int>(ErrorCode::MAX_VALUE)) {
        return std::error_condition(value, *this);
      }

      return std::make_error_condition(std::errc::invalid_argument);
    }
  } error_category;

  std::error_code MakeError(ErrorCode error_code) {
    return std::error_code(static_cast<int>(error_code), error_category);
  }

  std::error_code MaybeAddVertex() {
    current_vertex_index_ += 1u;

    if (current_vertex_index_ == handle_vertex_index_) {
      AddVertex(xyz_, normals_, uvs_);
      current_vertex_index_ = 0u;
    }

    return std::error_code();
  }

  template <size_t index, typename T>
  std::error_code AddPosition(T value) {
    xyz_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(xyz_[index])) {
      if constexpr (index == 0) {
        return MakeError(ErrorCode::VERTEX_X_NOT_FINITE);
      } else if constexpr (index == 1) {
        return MakeError(ErrorCode::VERTEX_Y_NOT_FINITE);
      } else {
        return MakeError(ErrorCode::VERTEX_Z_NOT_FINITE);
      }
    }

    return MaybeAddVertex();
  }

  template <size_t index, typename T>
  std::error_code AddNormal(T value) {
    normals_storage_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(normals_storage_[index])) {
      if constexpr (index == 0) {
        return MakeError(ErrorCode::NORMAL_X_NOT_FINITE);
      } else if constexpr (index == 1) {
        return MakeError(ErrorCode::NORMAL_Y_NOT_FINITE);
      } else {
        return MakeError(ErrorCode::NORMAL_Z_NOT_FINITE);
      }
    }

    return MaybeAddVertex();
  }

  template <size_t index, typename T>
  std::error_code AddUV(T value) {
    uv_storage_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(uv_storage_[index])) {
      if constexpr (index == 0) {
        return MakeError(ErrorCode::TEXTURE_U_NOT_FINITE);
      } else {
        return MakeError(ErrorCode::TEXTURE_V_NOT_FINITE);
      }
    }

    return MaybeAddVertex();
  }

  template <typename T>
  std::error_code ValidateVertexIndex(T index) {
    if constexpr (std::is_signed<T>::value) {
      if (index < 0) {
        return MakeError(ErrorCode::VERTEX_INDEX_OUT_OF_RANGE);
      }
    }

    uintmax_t unsigned_index = static_cast<uintmax_t>(index);

    if (unsigned_index >= num_vertices_) {
      return MakeError(ErrorCode::VERTEX_INDEX_OUT_OF_RANGE);
    }

    if (unsigned_index >
        static_cast<uintmax_t>(std::numeric_limits<FaceIndexType>::max())) {
      return MakeError(ErrorCode::VERTEX_INDEX_OUT_OF_RANGE);
    }

    return std::error_code();
  }

  template <typename T>
  std::error_code AddVertexIndices(std::span<const T> value) {
    if (value.size() >= 3) {
      if (std::error_code error = ValidateVertexIndex(value[0]); error) {
        return error;
      }

      if (std::error_code error = ValidateVertexIndex(value[1]); error) {
        return error;
      }

      std::array<FaceIndexType, 3> faces;
      faces[0] = static_cast<FaceIndexType>(value[0]);
      for (size_t i = 2u; i < value.size(); i++) {
        if (std::error_code error = ValidateVertexIndex(value[i]); error) {
          return error;
        }

        faces[1] = static_cast<FaceIndexType>(value[i - 1u]);
        faces[2] = static_cast<FaceIndexType>(value[i]);
        AddFace(faces);
      }
    }

    return std::error_code();
  }

  static const PropertyCallback *LookupProperty(
      const std::map<std::string, std::map<std::string, PropertyCallback>>
          &callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto element_iter = callbacks.find(element_name);
    if (element_iter == callbacks.end()) {
      return nullptr;
    }

    auto property_iter = element_iter->second.find(property_name);
    if (property_iter == element_iter->second.end()) {
      return nullptr;
    }

    return &property_iter->second;
  }

  template <size_t index>
  std::expected<std::optional<PropertyCallback>, std::error_code>
  LocationPropertyIndex(
      const std::map<std::string, std::map<std::string, PropertyCallback>>
          &callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property = LookupProperty(callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return FloatPropertyCallback([this](float value) {
          return TriangleMeshReader::AddPosition<index, float>(value);
        });
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return DoublePropertyCallback([this](double value) {
          return TriangleMeshReader::AddPosition<index, double>(value);
        });
      }

      return std::unexpected(MakeError(ErrorCode::INVALID_VERTEX_TYPE));
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<PropertyCallback>, std::error_code>
  NormalPropertyIndex(
      const std::map<std::string, std::map<std::string, PropertyCallback>>
          &callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property = LookupProperty(callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return FloatPropertyCallback([this](float value) {
          return TriangleMeshReader::AddNormal<index, float>(value);
        });
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return DoublePropertyCallback([this](double value) {
          return TriangleMeshReader::AddNormal<index, double>(value);
        });
      }

      return std::unexpected(MakeError(ErrorCode::INVALID_NORMAL_TYPE));
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string, PropertyCallback>>,
                std::error_code>
  UVPropertyIndex(
      const std::map<std::string, std::map<std::string, PropertyCallback>>
          &callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property = LookupProperty(callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return std::make_pair(
            property_name, FloatPropertyCallback([this](float value) {
              return TriangleMeshReader::AddUV<index, float>(value);
            }));
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return std::make_pair(
            property_name, DoublePropertyCallback([this](double value) {
              return TriangleMeshReader::AddUV<index, double>(value);
            }));
      }

      return std::unexpected(
          MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE));
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string, PropertyCallback>>,
                std::error_code>
  UVPropertyIndex(
      std::map<std::string, std::map<std::string, PropertyCallback>> &callbacks,
      const std::string &element_name,
      const std::vector<std::string> &property_names) {
    for (const auto &property_name : property_names) {
      auto face_property_index =
          UVPropertyIndex<index>(callbacks, element_name, property_name);
      if (!face_property_index || *face_property_index) {
        return face_property_index;
      }
    }

    return std::nullopt;
  }

  std::expected<std::optional<PropertyCallback>, std::error_code>
  FacePropertyIndex(
      std::map<std::string, std::map<std::string, PropertyCallback>> &callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property = LookupProperty(callbacks, element_name, property_name);

    if (property) {
      auto *int8_callback = std::get_if<CharPropertyListCallback>(property);
      if (int8_callback) {
        return CharPropertyListCallback([this](std::span<const int8_t> value) {
          return TriangleMeshReader::AddVertexIndices<int8_t>(value);
        });
      }

      auto *uint8_callback = std::get_if<UCharPropertyListCallback>(property);
      if (uint8_callback) {
        return UCharPropertyListCallback(
            [this](std::span<const uint8_t> value) {
              return TriangleMeshReader::AddVertexIndices<uint8_t>(value);
            });
      }

      auto *int16_callback = std::get_if<ShortPropertyListCallback>(property);
      if (int16_callback) {
        return ShortPropertyListCallback(
            [this](std::span<const int16_t> value) {
              return TriangleMeshReader::AddVertexIndices<int16_t>(value);
            });
      }

      auto *uint16_callback = std::get_if<UShortPropertyListCallback>(property);
      if (uint16_callback) {
        return UShortPropertyListCallback(
            [this](std::span<const uint16_t> value) {
              return TriangleMeshReader::AddVertexIndices<uint16_t>(value);
            });
      }

      auto *int32_callback = std::get_if<IntPropertyListCallback>(property);
      if (int32_callback) {
        return IntPropertyListCallback([this](std::span<const int32_t> value) {
          return TriangleMeshReader::AddVertexIndices<int32_t>(value);
        });
      }

      auto *uint32_callback = std::get_if<UIntPropertyListCallback>(property);
      if (uint32_callback) {
        return UIntPropertyListCallback(
            [this](std::span<const uint32_t> value) {
              return TriangleMeshReader::AddVertexIndices<uint32_t>(value);
            });
      }

      return std::unexpected(MakeError(ErrorCode::INVALID_VERTEX_INDEX_TYPE));
    }

    return std::nullopt;
  }

  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>> &callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) final {
    Start();

    auto x = LocationPropertyIndex<0>(callbacks, "vertex", "x");
    if (!x) {
      return x.error();
    }

    auto y = LocationPropertyIndex<1>(callbacks, "vertex", "y");
    if (!y) {
      return y.error();
    }

    auto z = LocationPropertyIndex<2>(callbacks, "vertex", "z");
    if (!z) {
      return z.error();
    }

    auto nx = NormalPropertyIndex<0>(callbacks, "vertex", "nx");
    if (!nx) {
      return nx.error();
    }

    auto ny = NormalPropertyIndex<1>(callbacks, "vertex", "ny");
    if (!ny) {
      return ny.error();
    }

    auto nz = NormalPropertyIndex<2>(callbacks, "vertex", "nz");
    if (!nz) {
      return nz.error();
    }

    auto u = UVPropertyIndex<0>(callbacks, "vertex",
                                {"u", "s", "texture_u", "texture_s"});
    if (!u) {
      return u.error();
    }

    auto v = UVPropertyIndex<1>(callbacks, "vertex",
                                {"v", "t", "texture_v", "texture_t"});
    if (!v) {
      return v.error();
    }

    auto vertex_indices =
        FacePropertyIndex(callbacks, "face", "vertex_indices");
    if (!vertex_indices) {
      return vertex_indices.error();
    }

    if (!*x || !*y || !*z) {
      return MakeError(ErrorCode::MISSING_VERTEX_DIMENSION);
    }

    num_vertices_ = num_element_instances.find("vertex")->second;

    callbacks["vertex"]["x"] = **x;
    callbacks["vertex"]["y"] = **y;
    callbacks["vertex"]["z"] = **z;

    if (*nx && *ny && *nz) {
      normals_ = &normals_storage_;
      callbacks["vertex"]["nx"] = **nx;
      callbacks["vertex"]["ny"] = **ny;
      callbacks["vertex"]["nz"] = **nz;
    } else {
      normals_ = nullptr;
    }

    if (*u && *v) {
      uvs_ = &uv_storage_;
      callbacks["vertex"][(*u)->first] = (*u)->second;
      callbacks["vertex"][(*v)->first] = (*v)->second;
    } else {
      uvs_ = nullptr;
    }

    handle_vertex_index_ = callbacks["vertex"].size();
    current_vertex_index_ = 0u;

    if (!*vertex_indices) {
      return MakeError(ErrorCode::MISSING_VERTEX_INDICES);
    }

    callbacks["face"]["vertex_indices"] = **vertex_indices;

    return std::error_code();
  }

  uintmax_t num_vertices_;
  size_t handle_vertex_index_;
  size_t current_vertex_index_;

  std::array<NormalType, 3> *normals_ = nullptr;
  std::array<UVType, 2> *uvs_ = nullptr;

  std::array<LocationType, 3> xyz_ = {0.0, 0.0, 0.0};
  std::array<NormalType, 3> normals_storage_ = {0.0, 0.0, 0.0};
  std::array<UVType, 2> uv_storage_ = {0.0, 0.0};
};

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
TriangleMeshReader<LocationType, NormalType, UVType,
                   FaceIndexType>::ErrorCategory
    TriangleMeshReader<LocationType, NormalType, UVType,
                       FaceIndexType>::error_category;

}  // namespace plyodine

#endif  // _PLYODINE_READERS_TRIANGLE_MESH_READER_
#ifndef _PLYODINE_READERS_TRIANGLE_MESH_READER_
#define _PLYODINE_READERS_TRIANGLE_MESH_READER_

#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>

#include "plyodine/ply_reader.h"

namespace plyodine {

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
class TriangleMeshReader : public PlyReader {
 public:
  virtual void Start() = 0;

  virtual void AddVertex(const std::array<LocationType, 3> &position,
                         const std::array<NormalType, 3> *maybe_normal,
                         const std::array<UVType, 2> *maybe_uv) = 0;

  virtual void AddFace(const std::array<FaceIndexType, 3> &face) = 0;

 private:
  std::expected<void, std::string> MaybeAddVertex() {
    current_vertex_index_ += 1u;

    if (current_vertex_index_ == handle_vertex_index_) {
      if (normals_ && normals_storage_[0] == 0.0 &&
          normals_storage_[1] == 0.0 && normals_storage_[2] == 0.0) {
        return std::unexpected("Input contained a zero length surface normal");
      }

      AddVertex(xyz_, normals_, uvs_);

      current_vertex_index_ = 0u;
    }

    return std::expected<void, std::string>();
  }

  template <size_t index, typename T>
  std::expected<void, std::string> AddPosition(const std::string &element_name,
                                               size_t element_index,
                                               const std::string &property_name,
                                               size_t property_index,
                                               uint64_t instance, T value) {
    xyz_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(xyz_[index])) {
      if constexpr (index == 0) {
        return std::unexpected("Input contained a non-finite value for x");
      } else if constexpr (index == 1) {
        return std::unexpected("Input contained a non-finite value for y");
      } else {
        return std::unexpected("Input contained a non-finite value for z");
      }
    }

    return MaybeAddVertex();
  }

  template <size_t index, typename T>
  std::expected<void, std::string> AddNormal(const std::string &element_name,
                                             size_t element_index,
                                             const std::string &property_name,
                                             size_t property_index,
                                             uint64_t instance, T value) {
    normals_storage_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(normals_storage_[index])) {
      if constexpr (index == 0) {
        return std::unexpected("Input contained a non-finite value for nx");
      } else if constexpr (index == 1) {
        return std::unexpected("Input contained a non-finite value for ny");
      } else {
        return std::unexpected("Input contained a non-finite value for nz");
      }
    }

    return MaybeAddVertex();
  }

  template <size_t index, typename T>
  std::expected<void, std::string> AddUV(const std::string &element_name,
                                         size_t element_index,
                                         const std::string &property_name,
                                         size_t property_index,
                                         uint64_t instance, T value) {
    uv_storage_[index] = static_cast<LocationType>(value);

    if (!std::isfinite(uv_storage_[index])) {
      if constexpr (index == 0) {
        return std::unexpected("Input contained a non-finite value for u");
      } else {
        return std::unexpected("Input contained a non-finite value for v");
      }
    }

    return MaybeAddVertex();
  }

  template <typename T>
  std::expected<void, std::string> ValidateVertexIndex(T index) {
    static const char *message = "A vertex index was out of range";

    if constexpr (std::is_signed<T>::value) {
      if (index < 0) {
        return std::unexpected(message);
      }
    }

    uint64_t unsigned_index = static_cast<uint64_t>(index);

    if (unsigned_index >= num_vertices_) {
      return std::unexpected(message);
    }

    if (unsigned_index >
        static_cast<uint64_t>(std::numeric_limits<FaceIndexType>::max())) {
      return std::unexpected(message);
    }

    return std::expected<void, std::string>();
  }

  template <typename T>
  std::expected<void, std::string> AddVertexIndices(
      const std::string &element_name, size_t element_index,
      const std::string &property_name, size_t property_index,
      uint64_t instance, std::span<const T> value) {
    if (value.size() >= 3) {
      auto v0_valid = ValidateVertexIndex(value[0]);
      if (!v0_valid) {
        return v0_valid;
      }

      auto v1_valid = ValidateVertexIndex(value[1]);
      if (!v1_valid) {
        return v1_valid;
      }

      std::array<FaceIndexType, 3> faces;
      faces[0] = static_cast<FaceIndexType>(value[0]);
      for (size_t i = 2u; i < value.size(); i++) {
        auto vn_valid = ValidateVertexIndex(value[i]);
        if (!vn_valid) {
          return vn_valid;
        }

        faces[1] = static_cast<FaceIndexType>(value[i - 1u]);
        faces[2] = static_cast<FaceIndexType>(value[i]);
        AddFace(faces);
      }
    }

    return std::expected<void, std::string>();
  }

  static const Callback *LookupProperty(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto element_iter = property_callbacks.find(element_name);
    if (element_iter == property_callbacks.end()) {
      return nullptr;
    }

    auto property_iter = element_iter->second.second.find(property_name);
    if (property_iter == element_iter->second.second.end()) {
      return nullptr;
    }

    return &property_iter->second;
  }

  template <size_t index>
  std::expected<std::optional<Callback>, std::string> LocationPropertyIndex(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property =
        LookupProperty(property_callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return FloatPropertyCallback(
            &TriangleMeshReader::AddPosition<index, float>);
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return DoublePropertyCallback(
            &TriangleMeshReader::AddPosition<index, double>);
      }

      return std::unexpected(
          "The type of properties x, y, and z, on vertex elements must be "
          "either float or double");
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<Callback>, std::string> NormalPropertyIndex(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property =
        LookupProperty(property_callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return FloatPropertyCallback(
            &TriangleMeshReader::AddNormal<index, float>);
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return DoublePropertyCallback(
            &TriangleMeshReader::AddNormal<index, double>);
      }

      return std::unexpected(
          "The type of properties nx, ny, and nz, on vertex elements must be "
          "either float or double");
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string, Callback>>, std::string>
  UVPropertyIndex(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property =
        LookupProperty(property_callbacks, element_name, property_name);

    if (property) {
      auto *float_callback = std::get_if<FloatPropertyCallback>(property);
      if (float_callback) {
        return std::make_pair(
            property_name,
            FloatPropertyCallback(&TriangleMeshReader::AddUV<index, float>));
      }

      auto *double_callback = std::get_if<DoublePropertyCallback>(property);
      if (double_callback) {
        return std::make_pair(
            property_name,
            DoublePropertyCallback(&TriangleMeshReader::AddUV<index, double>));
      }

      return std::unexpected(
          "The type of properties texture_s, texture_t, texture_u, texture_v, "
          "s, t, u, and v on vertex elements must be either float or double");
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string, Callback>>, std::string>
  UVPropertyIndex(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name,
      const std::vector<std::string> &property_names) {
    for (const auto &property_name : property_names) {
      auto face_property_index = UVPropertyIndex<index>(
          property_callbacks, element_name, property_name);
      if (!face_property_index || *face_property_index) {
        return face_property_index;
      }
    }

    return std::nullopt;
  }

  std::expected<std::optional<Callback>, std::string> FacePropertyIndex(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::string &element_name, const std::string &property_name) {
    auto property =
        LookupProperty(property_callbacks, element_name, property_name);

    if (property) {
      auto *int8_callback = std::get_if<Int8PropertyListCallback>(property);
      if (int8_callback) {
        return Int8PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<int8_t>);
      }

      auto *uint8_callback = std::get_if<UInt8PropertyListCallback>(property);
      if (uint8_callback) {
        return UInt8PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<uint8_t>);
      }

      auto *int16_callback = std::get_if<Int16PropertyListCallback>(property);
      if (int16_callback) {
        return Int16PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<int16_t>);
      }

      auto *uint16_callback = std::get_if<UInt16PropertyListCallback>(property);
      if (uint16_callback) {
        return UInt16PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<uint16_t>);
      }

      auto *int32_callback = std::get_if<Int32PropertyListCallback>(property);
      if (int32_callback) {
        return Int32PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<int32_t>);
      }

      auto *uint32_callback = std::get_if<UInt32PropertyListCallback>(property);
      if (uint32_callback) {
        return UInt32PropertyListCallback(
            &TriangleMeshReader::AddVertexIndices<uint32_t>);
      }

      return std::unexpected(
          "The type of property vertex_indices on face elements must be an "
          "integral list type");
    }

    return std::nullopt;
  }

  std::expected<void, std::string> Start(
      std::map<std::string,
               std::pair<uint64_t, std::map<std::string, Callback>>>
          &property_callbacks,
      const std::vector<std::string> &comments,
      const std::vector<std::string> &obj_infos) final {
    Start();

    auto x = LocationPropertyIndex<0>(property_callbacks, "vertex", "x");
    if (!x) {
      return std::unexpected(x.error());
    }

    auto y = LocationPropertyIndex<1>(property_callbacks, "vertex", "y");
    if (!y) {
      return std::unexpected(y.error());
    }

    auto z = LocationPropertyIndex<2>(property_callbacks, "vertex", "z");
    if (!z) {
      return std::unexpected(z.error());
    }

    auto nx = NormalPropertyIndex<0>(property_callbacks, "vertex", "nx");
    if (!nx) {
      return std::unexpected(nx.error());
    }

    auto ny = NormalPropertyIndex<1>(property_callbacks, "vertex", "ny");
    if (!ny) {
      return std::unexpected(ny.error());
    }

    auto nz = NormalPropertyIndex<2>(property_callbacks, "vertex", "nz");
    if (!nz) {
      return std::unexpected(nz.error());
    }

    auto u = UVPropertyIndex<0>(property_callbacks, "vertex",
                                {"u", "s", "texture_u", "texture_s"});
    if (!u) {
      return std::unexpected(u.error());
    }

    auto v = UVPropertyIndex<1>(property_callbacks, "vertex",
                                {"v", "t", "texture_v", "texture_t"});
    if (!v) {
      return std::unexpected(v.error());
    }

    auto vertex_indices =
        FacePropertyIndex(property_callbacks, "face", "vertex_indices");
    if (!vertex_indices) {
      return std::unexpected(vertex_indices.error());
    }

    if (!*x || !*y || !*z) {
      return std::unexpected("Element vertex must have properties x, y, and z");
    }

    num_vertices_ = property_callbacks.at("vertex").first;

    property_callbacks["vertex"].second["x"] = **x;
    property_callbacks["vertex"].second["y"] = **y;
    property_callbacks["vertex"].second["z"] = **z;

    if (*nx && *ny && *nz) {
      normals_ = &normals_storage_;
      property_callbacks["vertex"].second["nx"] = **nx;
      property_callbacks["vertex"].second["ny"] = **ny;
      property_callbacks["vertex"].second["nz"] = **nz;
    } else {
      normals_ = nullptr;
    }

    if (*u && *v) {
      uvs_ = &uv_storage_;
      property_callbacks["vertex"].second[(*u)->first] = (*u)->second;
      property_callbacks["vertex"].second[(*v)->first] = (*v)->second;
    } else {
      uvs_ = nullptr;
    }

    handle_vertex_index_ = property_callbacks["vertex"].second.size();
    current_vertex_index_ = 0u;

    if (!*vertex_indices) {
      return std::unexpected("Element face must have property vertex_indices");
    }

    property_callbacks["face"].second["vertex_indices"] = **vertex_indices;

    return std::expected<void, std::string>();
  }

 private:
  uint64_t num_vertices_;
  size_t handle_vertex_index_;
  size_t current_vertex_index_;

  std::array<NormalType, 3> *normals_ = nullptr;
  std::array<UVType, 2> *uvs_ = nullptr;

  std::array<LocationType, 3> xyz_ = {0.0, 0.0, 0.0};
  std::array<NormalType, 3> normals_storage_ = {0.0, 0.0, 0.0};
  std::array<UVType, 2> uv_storage_ = {0.0, 0.0};
};

}  // namespace plyodine

#endif  // _PLYODINE_READERS_TRIANGLE_MESH_READER_
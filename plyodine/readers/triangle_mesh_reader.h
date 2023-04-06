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
  std::expected<void, std::string_view> MaybeAddVertex() {
    current_vertex_index_ += 1u;

    if (current_vertex_index_ == handle_vertex_index_) {
      if (normals_ && normals_storage_[0] == 0.0 &&
          normals_storage_[1] == 0.0 && normals_storage_[2] == 0.0) {
        return std::unexpected("Input contained a zero length surface normal");
      }

      AddVertex(xyz_, normals_, uvs_);

      current_vertex_index_ = 0u;
    }

    return std::expected<void, std::string_view>();
  }

  template <size_t index, typename T>
  std::expected<void, std::string_view> AddPosition(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index, uint64_t instance,
      T value) {
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
  std::expected<void, std::string_view> AddNormal(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index, uint64_t instance,
      T value) {
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
  std::expected<void, std::string_view> AddUV(std::string_view element_name,
                                              size_t element_index,
                                              std::string_view property_name,
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
  std::expected<void, std::string_view> ValidateVertexIndex(T index) {
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

    return std::expected<void, std::string_view>();
  }

  template <typename T>
  std::expected<void, std::string_view> AddVertexIndices(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index, uint64_t instance,
      std::span<const T> value) {
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

    return std::expected<void, std::string_view>();
  }

  static const PropertyType *LookupProperty(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto element_iter = properties.find(element_name);
    if (element_iter == properties.end()) {
      return nullptr;
    }

    auto property_iter = element_iter->second.second.find(property_name);
    if (property_iter == element_iter->second.second.end()) {
      return nullptr;
    }

    return &property_iter->second;
  }

  template <size_t index>
  std::expected<std::optional<PlyReader::Callback>, std::string_view>
  LocationPropertyIndex(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property) {
      switch (*property) {
        case PropertyType::FLOAT:
          return FloatPropertyCallback(
              &TriangleMeshReader::AddPosition<index, FloatProperty>);
        case PropertyType::DOUBLE:
          return DoublePropertyCallback(
              &TriangleMeshReader::AddPosition<index, DoubleProperty>);
        default:
          return std::unexpected(
              "The type of properties x, y, and z, on vertex elements must be "
              "either float or double");
      }
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<PlyReader::Callback>, std::string_view>
  NormalPropertyIndex(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property) {
      switch (*property) {
        case PropertyType::FLOAT:
          return FloatPropertyCallback(
              &TriangleMeshReader::AddNormal<index, FloatProperty>);
        case PropertyType::DOUBLE:
          return DoublePropertyCallback(
              &TriangleMeshReader::AddNormal<index, DoubleProperty>);
        default:
          return std::unexpected(
              "The type of properties nx, ny, and nz, on vertex elements must "
              "be either float or double");
      }
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string_view, PlyReader::Callback>>,
                std::string_view>
  UVPropertyIndex(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property) {
      switch (*property) {
        case PropertyType::FLOAT:
          return std::make_pair(
              property_name,
              FloatPropertyCallback(
                  &TriangleMeshReader::AddUV<index, FloatProperty>));
        case PropertyType::DOUBLE:
          return std::make_pair(
              property_name,
              DoublePropertyCallback(
                  &TriangleMeshReader::AddUV<index, DoubleProperty>));
        default:
          return std::unexpected(
              "The type of properties texture_s, texture_t, texture_u, "
              "texture_v, s, t, u, and v on vertex elements must be either "
              "float or double");
      }
    }

    return std::nullopt;
  }

  template <size_t index>
  std::expected<std::optional<std::pair<std::string_view, PlyReader::Callback>>,
                std::string_view>
  UVPropertyIndex(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      std::span<const std::string_view> property_names) {
    for (const auto &property_name : property_names) {
      auto face_property_index =
          UVPropertyIndex<index>(properties, element_name, property_name);
      if (!face_property_index || *face_property_index) {
        return face_property_index;
      }
    }

    return std::nullopt;
  }

  std::expected<std::optional<PlyReader::Callback>, std::string_view>
  FacePropertyIndex(
      const std::map<
          std::string_view,
          std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property) {
      switch (*property) {
        case PropertyType::INT8_LIST:
          return Int8PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<int8_t>);
        case PropertyType::UINT8_LIST:
          return UInt8PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<uint8_t>);
        case PropertyType::INT16_LIST:
          return Int16PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<int16_t>);
        case PropertyType::UINT16_LIST:
          return UInt16PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<uint16_t>);
        case PropertyType::INT32_LIST:
          return Int32PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<int32_t>);
        case PropertyType::UINT32_LIST:
          return UInt32PropertyListCallback(
              &TriangleMeshReader::AddVertexIndices<uint32_t>);
        default:
          return std::unexpected(
              "The type of property vertex_indices on face elements must be an "
              "integral list type");
      }
    }

    return std::nullopt;
  }

  std::expected<
      std::map<std::string_view, std::map<std::string_view, Callback>>,
      std::string_view>
  Start(const std::map<
            std::string_view,
            std::pair<uint64_t, std::map<std::string_view, PropertyType>>>
            &properties,
        const std::vector<std::string> &comments,
        const std::vector<std::string> &obj_infos) final {
    Start();

    auto x = LocationPropertyIndex<0>(properties, "vertex", "x");
    if (!x) {
      return std::unexpected(x.error());
    }

    auto y = LocationPropertyIndex<1>(properties, "vertex", "y");
    if (!y) {
      return std::unexpected(y.error());
    }

    auto z = LocationPropertyIndex<2>(properties, "vertex", "z");
    if (!z) {
      return std::unexpected(z.error());
    }

    auto nx = NormalPropertyIndex<0>(properties, "vertex", "nx");
    if (!nx) {
      return std::unexpected(nx.error());
    }

    auto ny = NormalPropertyIndex<1>(properties, "vertex", "ny");
    if (!ny) {
      return std::unexpected(ny.error());
    }

    auto nz = NormalPropertyIndex<2>(properties, "vertex", "nz");
    if (!nz) {
      return std::unexpected(nz.error());
    }

    auto u = UVPropertyIndex<0>(properties, "vertex",
                                {{"u", "s", "texture_u", "texture_s"}});
    if (!u) {
      return std::unexpected(u.error());
    }

    auto v = UVPropertyIndex<1>(properties, "vertex",
                                {{"v", "t", "texture_v", "texture_t"}});
    if (!v) {
      return std::unexpected(v.error());
    }

    auto vertex_indices =
        FacePropertyIndex(properties, "face", "vertex_indices");
    if (!vertex_indices) {
      return std::unexpected(vertex_indices.error());
    }

    if (!*x || !*y || !*z) {
      return std::unexpected("Element vertex must have properties x, y, and z");
    }

    num_vertices_ = properties.at("vertex").first;

    std::map<std::string_view, std::map<std::string_view, Callback>> result;
    result["vertex"]["x"] = **x;
    result["vertex"]["y"] = **y;
    result["vertex"]["z"] = **z;

    if (*nx && *ny && *nz) {
      normals_ = &normals_storage_;
      result["vertex"]["nx"] = **nx;
      result["vertex"]["ny"] = **ny;
      result["vertex"]["nz"] = **nz;
    } else {
      normals_ = nullptr;
    }

    if (*u && *v) {
      uvs_ = &uv_storage_;
      result["vertex"][(*u)->first] = (*u)->second;
      result["vertex"][(*v)->first] = (*v)->second;
    } else {
      uvs_ = nullptr;
    }

    handle_vertex_index_ = result["vertex"].size();
    current_vertex_index_ = 0u;

    if (!*vertex_indices) {
      return std::unexpected("Element face must have property vertex_indices");
    }

    result["face"]["vertex_indices"] = **vertex_indices;

    return std::expected<
        std::map<std::string_view, std::map<std::string_view, Callback>>,
        std::string_view>(result);
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
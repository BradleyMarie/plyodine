#ifndef _PLYODINE_READERS_TRIANGLE_MESH_READER_
#define _PLYODINE_READERS_TRIANGLE_MESH_READER_

#include <cmath>
#include <concepts>
#include <functional>
#include <type_traits>

#include "plyodine/ply_reader.h"

namespace plyodine {

template <std::floating_point LocationType, std::floating_point NormalType,
          std::floating_point UVType, std::integral FaceIndexType>
class TriangleMeshReader : public PlyReader {
 public:
  virtual void Start() = 0;

  virtual void Handle(LocationType position[3], NormalType maybe_normals[3],
                      UVType maybe_uv[2]) = 0;

  virtual void Handle(FaceIndexType face[3]) = 0;

 private:
  template <typename T>
  std::expected<void, std::string_view> Handle(
      const std::vector<std::function<void(T)>> &parse_functions,
      size_t property_index, T value) {
    std::expected<void, std::string_view> result;
    if (property_index < parse_functions.size() &&
        parse_functions[property_index]) {
      result = parse_functions[property_index](value);
    }

    if (property_index == parse_functions.size() - 1u) {
      Handle(xyz_, normals_, uvs_);
    }

    return result;
  }

  struct HandleX {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->xyz_[0] = static_cast<LocationType>(value);
          if (!std::isfinite(reader->xyz_[0])) {
            return std::unexpected("Input contained an non-finite value for x");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleY {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->xyz_[1] = static_cast<LocationType>(value);
          if (!std::isfinite(reader->xyz_[1])) {
            return std::unexpected("Input contained an non-finite value for y");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleZ {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->xyz_[2] = static_cast<LocationType>(value);
          if (!std::isfinite(reader->xyz_[2])) {
            return std::unexpected("Input contained an non-finite value for z");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleNX {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->normals_storage_[0] = static_cast<NormalType>(value);
          if (!std::isfinite(reader->normals_storage_[0])) {
            return std::unexpected("Input contained an non-finite value for nx");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleNY {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->normals_storage_[1] = static_cast<NormalType>(value);
          if (!std::isfinite(reader->normals_storage_[1])) {
            return std::unexpected("Input contained an non-finite value for ny");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleNZ {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->normals_storage_[2] = static_cast<NormalType>(value);
          if (!std::isfinite(reader->normals_storage_[2])) {
            return std::unexpected("Input contained an non-finite value for nz");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleU {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->uv_storage_[0] = static_cast<UVType>(value);
          if (!std::isfinite(reader->uv_storage_[0])) {
            return std::unexpected("Input contained an non-finite value for u");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleV {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader) const {
      return [reader](T value) {
        if constexpr (std::is_floating_point<T>::value) {
          reader->uv_storage_[1] = static_cast<UVType>(value);
          if (!std::isfinite(reader->uv_storage_[1])) {
            return std::unexpected("Input contained an non-finite value for v");
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  struct HandleVertexIndices {
    template <typename T>
    std::function<std::expected<void, std::string_view>(T)> GetCallback(
        TriangleMeshReader *reader, uint64_t num_vertices) const {
      return [reader, num_vertices](T value) {
        if constexpr (std::is_class<T>::value) {
          if (value.size() >= 3) {
            if (value[0] < 0 || value[0] >= num_vertices ||
                value[0] > std::numeric_limits<FaceIndexType>::max()) {
              return std::unexpected("A vertex index was out of range");
            }

            if (value[1] < 0 || value[1] >= num_vertices ||
                value[1] > std::numeric_limits<FaceIndexType>::max()) {
              return std::unexpected("A vertex index was out of range");
            }

            FaceIndexType faces[3];
            faces[0] = static_cast<FaceIndexType>(value[0]);
            for (size_t i = 0; i < value.size() - 2; i++) {
              if (value[i + 1] < 0 || value[i + 1] >= num_vertices ||
                  value[i + 1] > std::numeric_limits<FaceIndexType>::max()) {
                return std::unexpected("A vertex index was out of range");
              }

              faces[1] = static_cast<FaceIndexType>(value[i]);
              faces[2] = static_cast<FaceIndexType>(value[i + 1]);
              reader->Handle(faces);
            }
          }
        }
        return std::expected<void, std::string_view>();
      };
    }
  };

  void Resize(size_t size) {
    parse_int8_property_.resize(size, nullptr);
    parse_int8_property_list_.resize(size, nullptr);
    parse_uint8_property_.resize(size, nullptr);
    parse_uint8_property_list_.resize(size, nullptr);
    parse_int16_property_.resize(size, nullptr);
    parse_int16_property_list_.resize(size, nullptr);
    parse_uint16_property_.resize(size, nullptr);
    parse_uint16_property_list_.resize(size, nullptr);
    parse_int32_property_.resize(size, nullptr);
    parse_int32_property_list_.resize(size, nullptr);
    parse_uint32_property_.resize(size, nullptr);
    parse_uint32_property_list_.resize(size, nullptr);
    parse_float_property_.resize(size, nullptr);
    parse_float_property_list_.resize(size, nullptr);
    parse_double_property_.resize(size, nullptr);
    parse_double_property_list_.resize(size, nullptr);
  }

  void Clear() { Resize(0u); }

  void Grow(size_t new_size) {
    if (new_size < parse_int8_property_.size()) {
      Resize(new_size);
    }
  }

  template <typename T>
  void FillCallback(const std::pair<size_t, Property::Type> &entry) {
    Grow(entry.first);

    T object;
    switch (entry.second) {
      case Property::INT8:
        parse_int8_property_[entry.first] =
            object.template GetCallback<Int8Property>(this);
        break;
      case Property::INT8_LIST:
        parse_int8_property_list_[entry.first] =
            object.template GetCallback<Int8PropertyList>(this);
        break;
      case Property::UINT8:
        parse_uint8_property_[entry.first] =
            object.template GetCallback<UInt8Property>(this);
        break;
      case Property::UINT8_LIST:
        parse_uint8_property_list_[entry.first] =
            object.template GetCallback<UInt8PropertyList>(this);
        break;
      case Property::INT16:
        parse_int16_property_[entry.first] =
            object.template GetCallback<Int16Property>(this);
        break;
      case Property::INT16_LIST:
        parse_int16_property_list_[entry.first] =
            object.template GetCallback<Int16PropertyList>(this);
        break;
      case Property::UINT16:
        parse_uint16_property_[entry.first] =
            object.template GetCallback<UInt16Property>(this);
        break;
      case Property::UINT16_LIST:
        parse_uint16_property_list_[entry.first] =
            object.template GetCallback<UInt16PropertyList>(this);
        break;
      case Property::INT32:
        parse_int32_property_[entry.first] =
            object.template GetCallback<Int32Property>(this);
        break;
      case Property::INT32_LIST:
        parse_int32_property_list_[entry.first] =
            object.template GetCallback<Int32PropertyList>(this);
        break;
      case Property::UINT32:
        parse_uint32_property_[entry.first] =
            object.template GetCallback<UInt32Property>(this);
        break;
      case Property::UINT32_LIST:
        parse_uint32_property_list_[entry.first] =
            object.template GetCallback<UInt32PropertyList>(this);
        break;
      case Property::FLOAT:
        parse_float_property_[entry.first] =
            object.template GetCallback<FloatProperty>(this);
        break;
      case Property::FLOAT_LIST:
        parse_float_property_list_[entry.first] =
            object.template GetCallback<FloatPropertyList>(this);
        break;
      case Property::DOUBLE:
        parse_double_property_[entry.first] =
            object.template GetCallback<DoubleProperty>(this);
        break;
      case Property::DOUBLE_LIST:
        parse_double_property_list_[entry.first] =
            object.template GetCallback<DoublePropertyList>(this);
        break;
    }
  }

  static const std::pair<size_t, Property::Type> *LookupProperty(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
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

  static std::expected<const std::pair<size_t, Property::Type> *,
                       std::string_view>
  LocationPropertyIndex(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property && (property->second != Property::FLOAT ||
                     property->second != Property::DOUBLE)) {
      return std::unexpected(
          "The type of properties x, y, and z, on vertex elements must be "
          "either float or double");
    }

    return property;
  }

  static std::expected<const std::pair<size_t, Property::Type> *,
                       std::string_view>
  NormalPropertyIndex(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property && (property->second != Property::FLOAT ||
                     property->second != Property::DOUBLE)) {
      return std::unexpected(
          "The type of properties nx, ny, and nz, on vertex elements must be "
          "either float or double");
    }

    return property;
  }

  static std::expected<const std::pair<size_t, Property::Type> *,
                       std::string_view>
  UVPropertyIndex(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property && (property->second != Property::FLOAT ||
                     property->second != Property::DOUBLE)) {
      return std::unexpected(
          "The type of properties texture_s, texture_t, texture_u, texture_v, "
          "s, t, u, and v on vertex elements must be either float or double");
    }

    return property;
  }

  static std::expected<const std::pair<size_t, Property::Type> *,
                       std::string_view>
  UVPropertyIndex(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      const std::string_view &element_name,
      std::span<const std::string_view> property_names) {
    for (const auto &property_name : property_names) {
      auto face_property_index =
          UVPropertyIndex(properties, element_name, property_name);
      if (!face_property_index || *face_property_index) {
        return face_property_index;
      }
    }

    return nullptr;
  }

  static std::expected<const std::pair<size_t, Property::Type> *,
                       std::string_view>
  FacePropertyIndex(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      const std::string_view &element_name,
      const std::string_view &property_name) {
    auto property = LookupProperty(properties, element_name, property_name);

    if (property && (property->second != Property::INT8_LIST ||
                     property->second != Property::UINT8_LIST ||
                     property->second != Property::INT16_LIST ||
                     property->second != Property::UINT16_LIST ||
                     property->second != Property::INT32_LIST ||
                     property->second != Property::UINT32_LIST)) {
      return std::unexpected(
          "The type of property vertex_indices on face elements must be an "
          "integral list type");
    }

    return property;
  }

 public:
  std::expected<void, std::string_view> Start(
      const std::unordered_map<
          std::string_view,
          std::pair<uint64_t,
                    std::unordered_map<std::string_view,
                                       std::pair<size_t, Property::Type>>>>
          &properties,
      std::span<const std::string> comments,
      std::span<const std::string> obj_infos) final {
    Start();

    Clear();

    auto x = LocationPropertyIndex(properties, "vertex", "x");
    if (!x) {
      return std::unexpected(x.error());
    }

    auto y = LocationPropertyIndex(properties, "vertex", "y");
    if (!y) {
      return std::unexpected(y.error());
    }

    auto z = LocationPropertyIndex(properties, "vertex", "z");
    if (!z) {
      return std::unexpected(z.error());
    }

    auto nx = NormalPropertyIndex(properties, "vertex", "nx");
    if (!nx) {
      return std::unexpected(nx.error());
    }

    auto ny = NormalPropertyIndex(properties, "vertex", "ny");
    if (!ny) {
      return std::unexpected(ny.error());
    }

    auto nz = NormalPropertyIndex(properties, "vertex", "nz");
    if (!nz) {
      return std::unexpected(nz.error());
    }

    auto u = UVPropertyIndex(properties, "vertex",
                             {{"u", "s", "texture_u", "texture_s"}});
    if (!u) {
      return std::unexpected(u.error());
    }

    auto v = UVPropertyIndex(properties, "vertex",
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

    FillCallback<HandleX>(**x);
    FillCallback<HandleY>(**y);
    FillCallback<HandleZ>(**z);

    if (*nx && *ny && *nz) {
      normals_ = normals_storage_;
      FillCallback<HandleNX>(**nx);
      FillCallback<HandleNY>(**ny);
      FillCallback<HandleNZ>(**nz);
    } else {
      normals_ = nullptr;
    }

    if (*u && *v) {
      uvs_ = uv_storage_;
      FillCallback<HandleU>(**u);
      FillCallback<HandleV>(**v);
    } else {
      uvs_ = nullptr;
    }

    if (!*vertex_indices) {
      return std::unexpected("Element face must have property vertex_indices");
    }

    FillCallback<HandleVertexIndices>(**vertex_indices,
                                      properties.at("vertex").first);

    return std::expected<void, std::string_view>();
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int8Property value) final {
    return Handle(parse_int8_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int8PropertyList values) final {
    return Handle(parse_int8_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               UInt8Property value) final {
    return Handle(parse_uint8_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               UInt8PropertyList values) final {
    return Handle(parse_uint8_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int16Property value) final {
    return Handle(parse_int16_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int16PropertyList values) final {
    return Handle(parse_int16_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               UInt16Property value) final {
    return Handle(parse_uint16_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt16PropertyList values) final {
    return Handle(parse_uint16_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int32Property value) final {
    return Handle(parse_int32_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               Int32PropertyList values) final {
    return Handle(parse_int32_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               UInt32Property value) final {
    return Handle(parse_uint32_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt32PropertyList values) final {
    return Handle(parse_uint32_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               FloatProperty value) final {
    return Handle(parse_float_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               FloatPropertyList values) final {
    return Handle(parse_float_property_list_, property_index, values);
  }

  std::expected<void, std::string_view> Handle(std::string_view element_name,
                                               std::string_view property_name,
                                               size_t property_index,
                                               DoubleProperty value) final {
    return Handle(parse_double_property_, property_index, value);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, DoublePropertyList values) final {
    return Handle(parse_double_property_list_, property_index, values);
  }

 private:
  std::vector<
      std::function<std::expected<void, std::string_view>(Int8Property)>>
      parse_int8_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(Int8PropertyList)>>
      parse_int8_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt8Property)>>
      parse_uint8_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt8PropertyList)>>
      parse_uint8_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(Int16Property)>>
      parse_int16_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(Int16PropertyList)>>
      parse_int16_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt16Property)>>
      parse_uint16_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt16PropertyList)>>
      parse_uint16_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(Int32Property)>>
      parse_int32_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(Int32PropertyList)>>
      parse_int32_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt32Property)>>
      parse_uint32_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(UInt32PropertyList)>>
      parse_uint32_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(FloatProperty)>>
      parse_float_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(FloatPropertyList)>>
      parse_float_property_list_;
  std::vector<
      std::function<std::expected<void, std::string_view>(DoubleProperty)>>
      parse_double_property_;
  std::vector<
      std::function<std::expected<void, std::string_view>(DoublePropertyList)>>
      parse_double_property_list_;

  NormalType *normals_ = nullptr;
  UVType *uvs_ = nullptr;

  LocationType xyz_[3] = {0.0, 0.0, 0.0};
  NormalType normals_storage_[3] = {0.0, 0.0, 0.0};
  UVType uv_storage_[2] = {0.0, 0.0};
};

}  // namespace plyodine

#endif  // _PLYODINE_READERS_TRIANGLE_MESH_READER_
#ifndef _PLYODINE_READERS_TRIANGLE_MESH_READER_
#define _PLYODINE_READERS_TRIANGLE_MESH_READER_

#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <system_error>
#include <vector>

#include "plyodine/ply_reader.h"

namespace plyodine {

// A PLY reader that reads a PLY input as a triangle mesh. It is expected that
// most PLY readers with standard usage will derive from this class instead of
// implementing PlyReader directly.
//
// This reader is capable of reading the vertices, faces, normal, and texture
// coordinates from a model with automatic conversion in to the client's desired
// precision.
//
// The elements and properties this reader looks for in a model are as follows.
//
// element "vertex" - Required - The element representing a vertex
//   property [fp] "x" - Required - The vertex X coordinate
//   property [fp] "y" - Required - The vertex Y coordinate
//   property [fp] "z" - Required - The vertex Z coordinate
//   property [fp] "nx" - Optional - The vertex normal X length
//   property [fp] "ny" - Optional - The vertex normal Y length
//   property [fp] "nz" - Optional - The vertex normal Z length
//   property [fp] "texture_s" - Optional - The vertex texture U coordinate
//   property [fp] "texture_t" - Optional - The vertex texture V coordinate
//   property [fp] "texture_u" - Optional - The vertex texture U coordinate
//   property [fp] "texture_v" - Optional - The vertex texture V coordinate
//   property [fp] "s" - Optional - The vertex texture U coordinate
//   property [fp] "t" - Optional - The vertex texture V coordinate
//   property [fp] "u" - Optional - The vertex texture U coordinate
//   property [fp] "v" - Optional - The vertex texture V coordinate
//
// element "face" - Required - The element representing a face
//   property list [int] [int] "vertex_indices" - The vertex indices of the face
//
// `TriangleMeshReader` requires at least 3 vertex indices in order for each
// face and providing fewer than that will cause that face to be ignored. If
// more than 3 indices are provided, they will be interpreted as a triangle fan.
//
// For normal, which are optional, `TriangleMeshReader` will ignore the normal
// unless all three of the axes are present. Similarly for texture coordinates,
// `TriangleMeshReader` will ignore the coordinates unless both U and V are
// present. Additionally, in the event that multiple aliases are present for the
// same texture coordinate only one alias will be used for that coordinate and
// the others will be ignored. The exact alias selected is not defined.
//
// NOTE: `TriangleMeshReader` is flexible in the exact types used by the model
// for each property as long as the types match the integer/floating point
// requirements listed above.
//
// NOTE: The interface of this class is not yet fully stable and as such this
// class should be considered experimental. It is possible that breaking changes
// may be made to this class in the future which will not be reflected in the
// plyodine major version number.
template <std::floating_point PositionType = float,
          std::floating_point NormalType = float,
          std::floating_point UVType = float,
          std::unsigned_integral VertexIndexType = uint32_t>
  requires(sizeof(PositionType) <= sizeof(double) &&
           sizeof(NormalType) <= sizeof(double) &&
           sizeof(UVType) <= sizeof(double) &&
           sizeof(VertexIndexType) <= sizeof(uint32_t))
class TriangleMeshReader : public PlyReader {
 protected:
  // This function may be implemented by derived classes to identify the start
  // of parsing
  virtual void Start() {}

  // This function is implemented by derived classes in order to receive the
  // vertex data from the model.
  //
  // `position` - The X, Y, and Z coordinates of the vertex position.
  //
  // `maybe_normal` - If the model contains the properties required, the X, Y,
  // and Z length of the normal, otherwise nullptr.
  //
  // `maybe_uv` - If the model contains the properties required, the U and V
  // texture coordinates of the normal, otherwise nullptr.
  virtual void AddVertex(const std::array<PositionType, 3> &position,
                         const std::array<NormalType, 3> *maybe_normal,
                         const std::array<UVType, 2> *maybe_uv) = 0;

  // This function is implemented by derived classes in order to receive the
  // vertex indices of each triangle in the model.
  virtual void AddTriangle(
      const std::array<VertexIndexType, 3> &vertex_indices) = 0;

 private:
  enum class ErrorCode {
    MIN_VALUE = 1,
    MISSING_VERTEX_ELEMENT = 1,
    MISSING_FACE_ELEMENT = 2,
    MISSING_POSITION_DIMENSION = 3,
    INVALID_POSITION_TYPE = 4,
    MISSING_VERTEX_INDICES = 5,
    INVALID_VERTEX_INDEX_TYPE = 6,
    INVALID_NORMAL_TYPE = 7,
    INVALID_TEXTURE_COORDINATE_TYPE = 8,
    POSITION_NOT_FINITE = 9,
    POSITION_CONVERSION_FAILED = 10,
    VERTEX_INDEX_OUT_OF_RANGE = 11,
    VERTEX_INDEX_CONVERSION_UNDERFLOWED = 12,
    VERTEX_INDEX_CONVERSION_OVERFLOWED = 13,
    NORMAL_NOT_FINITE = 14,
    NORMAL_CONVERSION_FAILED = 15,
    TEXTURE_COORDINATE_NOT_FINITE = 16,
    TEXTURE_COORDINATE_CONVERSION_FAILED = 17,
    MAX_VALUE = 17,
  };

  static class ErrorCategory final : public std::error_category {
    const char *name() const noexcept override {
      return "plyodine::TriangleMeshReader";
    }

    std::string message(int condition) const override {
      ErrorCode error_code{condition};
      switch (error_code) {
        case ErrorCode::MISSING_VERTEX_ELEMENT:
          return "Element vertex not found";
        case ErrorCode::MISSING_FACE_ELEMENT:
          return "Element face not found";
        case ErrorCode::MISSING_POSITION_DIMENSION:
          return "Element vertex must have properties x, y, and z";
        case ErrorCode::INVALID_POSITION_TYPE:
          return "The type of properties x, y, and z, on vertex elements must "
                 "be either float or double";
        case ErrorCode::MISSING_VERTEX_INDICES:
          return "Element face must have property vertex_indices";
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
        case ErrorCode::POSITION_NOT_FINITE:
          return "Input contained a non-finite position";
        case ErrorCode::POSITION_CONVERSION_FAILED:
          return "Input contained a position that could not fit finitely into "
                 "its destination type";
        case ErrorCode::VERTEX_INDEX_OUT_OF_RANGE:
          return "A vertex index was out of range";
        case ErrorCode::VERTEX_INDEX_CONVERSION_UNDERFLOWED:
          return "A vertex index was negative";
        case ErrorCode::VERTEX_INDEX_CONVERSION_OVERFLOWED:
          return "A vertex index was too large to fit into its destination "
                 "type";
        case ErrorCode::NORMAL_NOT_FINITE:
          return "Input contained a non-finite normal";
        case ErrorCode::NORMAL_CONVERSION_FAILED:
          return "Input contained a normal that could not fit finitely into "
                 "its destination type";
        case ErrorCode::TEXTURE_COORDINATE_NOT_FINITE:
          return "Input contained a non-finite texture coordinate";
        case ErrorCode::TEXTURE_COORDINATE_CONVERSION_FAILED:
          return "Input contained a texture coordinate that could not fit "
                 "finitely into its destination type";
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

  static std::error_code MakeError(ErrorCode error_code) {
    return std::error_code(static_cast<int>(error_code), error_category);
  }

  static bool FloatingPointCallback(const PropertyCallback &callback) {
    return std::holds_alternative<FloatPropertyCallback>(callback) ||
           std::holds_alternative<DoublePropertyCallback>(callback);
  }

  std::error_code MaybeAddVertex() {
    current_vertex_index_ += 1u;

    if (current_vertex_index_ == handle_vertex_index_) {
      AddVertex(xyz_, normal_, uv_);
      current_vertex_index_ = 0u;
    }

    return std::error_code();
  }

  std::error_code AddVertexPositionCallback(
      std::map<std::string, PropertyCallback> &callbacks, size_t index) {
    static const std::string property_names[3] = {"x", "y", "z"};

    auto iter = callbacks.find(property_names[index]);
    if (iter == callbacks.end()) {
      return MakeError(ErrorCode::MISSING_POSITION_DIMENSION);
    }

    if (!FloatingPointCallback(iter->second)) {
      return MakeError(ErrorCode::INVALID_POSITION_TYPE);
    }

    iter->second = std::function<std::error_code(PositionType)>(
        [index, this](PositionType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(ErrorCode::POSITION_NOT_FINITE);
          }

          xyz_[index] = value;

          return MaybeAddVertex();
        });

    return std::error_code();
  }

  std::error_code AddVertexIndicesCallback(
      std::map<std::string, PropertyCallback> &callbacks,
      uintmax_t num_vertices) {
    auto iter = callbacks.find("vertex_indices");
    if (iter == callbacks.end()) {
      return MakeError(ErrorCode::MISSING_VERTEX_INDICES);
    }

    if (!std::holds_alternative<CharPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UCharPropertyListCallback>(iter->second) &&
        !std::holds_alternative<ShortPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UShortPropertyListCallback>(iter->second) &&
        !std::holds_alternative<IntPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UIntPropertyListCallback>(iter->second)) {
      return MakeError(ErrorCode::INVALID_VERTEX_INDEX_TYPE);
    }

    iter->second =
        std::function<std::error_code(std::span<const VertexIndexType>)>(
            [num_vertices, this](std::span<const VertexIndexType> indices) {
              if (indices.size() < 3) {
                return std::error_code();
              }

              if (num_vertices < indices[0] || num_vertices < indices[1]) {
                return MakeError(ErrorCode::VERTEX_INDEX_OUT_OF_RANGE);
              }

              std::array<VertexIndexType, 3> faces;
              faces[0] = static_cast<VertexIndexType>(indices[0]);
              for (size_t i = 2u; i < indices.size(); i++) {
                if (num_vertices < indices[i]) {
                  return MakeError(ErrorCode::VERTEX_INDEX_OUT_OF_RANGE);
                }

                faces[1] = static_cast<VertexIndexType>(indices[i - 1u]);
                faces[2] = static_cast<VertexIndexType>(indices[i]);

                if (faces[0] != faces[1] && faces[1] != faces[2] &&
                    faces[2] != faces[0]) {
                  AddTriangle(faces);
                }
              }

              return std::error_code();
            });

    return std::error_code();
  }

  void AddVertexNormalCallback(PropertyCallback &callbacks, size_t index) {
    callbacks = std::function<std::error_code(NormalType)>(
        [index, this](NormalType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(ErrorCode::NORMAL_NOT_FINITE);
          }

          normal_storage_[index] = value;

          return MaybeAddVertex();
        });
  }

  std::error_code AddVertexNormalCallbacks(
      std::map<std::string, PropertyCallback> &callbacks) {
    auto x_iter = callbacks.find("nx");
    auto y_iter = callbacks.find("ny");
    auto z_iter = callbacks.find("nz");

    bool success = true;
    if (x_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(x_iter->second)) {
      return MakeError(ErrorCode::INVALID_NORMAL_TYPE);
    }

    if (y_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(y_iter->second)) {
      return MakeError(ErrorCode::INVALID_NORMAL_TYPE);
    }

    if (z_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(z_iter->second)) {
      return MakeError(ErrorCode::INVALID_NORMAL_TYPE);
    }

    if (success) {
      AddVertexNormalCallback(x_iter->second, 0);
      AddVertexNormalCallback(y_iter->second, 1);
      AddVertexNormalCallback(z_iter->second, 2);

      normal_ = &normal_storage_;
      handle_vertex_index_ += 3;
    }

    return std::error_code();
  }

  void AddVertexUVCallback(PropertyCallback &callbacks, size_t index) {
    callbacks = std::function<std::error_code(UVType)>(
        [index, this](UVType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(ErrorCode::TEXTURE_COORDINATE_NOT_FINITE);
          }

          uv_storage_[index] = value;

          return MaybeAddVertex();
        });
  }

  std::error_code AddVertexUVCallbacks(
      std::map<std::string, PropertyCallback> &callbacks) {
    auto texture_s_iter = callbacks.find("texture_s");
    auto texture_t_iter = callbacks.find("texture_t");
    auto texture_u_iter = callbacks.find("texture_u");
    auto texture_v_iter = callbacks.find("texture_v");
    auto s_iter = callbacks.find("s");
    auto t_iter = callbacks.find("t");
    auto u_iter = callbacks.find("u");
    auto v_iter = callbacks.find("v");

    auto selected_u_iter = callbacks.end();
    if (texture_s_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_s_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_u_iter = texture_s_iter;
    }

    if (texture_u_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_u_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_u_iter = texture_u_iter;
    }

    if (s_iter != callbacks.end()) {
      if (!FloatingPointCallback(s_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_u_iter = s_iter;
    }

    if (u_iter != callbacks.end()) {
      if (!FloatingPointCallback(u_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_u_iter = u_iter;
    }

    auto selected_v_iter = callbacks.end();
    if (texture_t_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_t_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_v_iter = texture_t_iter;
    }

    if (texture_v_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_v_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_v_iter = texture_v_iter;
    }

    if (t_iter != callbacks.end()) {
      if (!FloatingPointCallback(t_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_v_iter = t_iter;
    }

    if (v_iter != callbacks.end()) {
      if (!FloatingPointCallback(v_iter->second)) {
        return MakeError(ErrorCode::INVALID_TEXTURE_COORDINATE_TYPE);
      }

      selected_v_iter = v_iter;
    }

    if (selected_u_iter != callbacks.end() &&
        selected_v_iter != callbacks.end()) {
      AddVertexUVCallback(selected_u_iter->second, 0);
      AddVertexUVCallback(selected_v_iter->second, 1);

      uv_ = &uv_storage_;
      handle_vertex_index_ += 2;
    }

    return std::error_code();
  }

  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>> &callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) final {
    current_vertex_index_ = 0;
    handle_vertex_index_ = 3;
    normal_ = nullptr;
    uv_ = nullptr;

    auto vertex_callbacks = callbacks.find("vertex");
    if (vertex_callbacks == callbacks.end()) {
      return MakeError(ErrorCode::MISSING_VERTEX_ELEMENT);
    }

    auto face_callbacks = callbacks.find("face");
    if (face_callbacks == callbacks.end()) {
      return MakeError(ErrorCode::MISSING_FACE_ELEMENT);
    }

    if (std::error_code error =
            AddVertexPositionCallback(vertex_callbacks->second, 0);
        error) {
      return error;
    }

    if (std::error_code error =
            AddVertexPositionCallback(vertex_callbacks->second, 1);
        error) {
      return error;
    }

    if (std::error_code error =
            AddVertexPositionCallback(vertex_callbacks->second, 2);
        error) {
      return error;
    }

    if (std::error_code error = AddVertexIndicesCallback(
            face_callbacks->second, num_element_instances["vertex"]);
        error) {
      return error;
    }

    if (std::error_code error =
            AddVertexNormalCallbacks(vertex_callbacks->second);
        error) {
      return error;
    }

    if (std::error_code error = AddVertexUVCallbacks(vertex_callbacks->second);
        error) {
      return error;
    }

    Start();

    return std::error_code();
  }

  std::error_code OnConversionFailure(const std::string &element,
                                      const std::string &property,
                                      ConversionFailureReason reason) override {
    if (reason == ConversionFailureReason::UNSIGNED_INTEGER_UNDERFLOW) {
      return MakeError(ErrorCode::VERTEX_INDEX_CONVERSION_UNDERFLOWED);
    }

    if (property == "x" || property == "y" || property == "z") {
      return MakeError(ErrorCode::POSITION_CONVERSION_FAILED);
    }

    if (property == "vertex_indices") {
      return MakeError(ErrorCode::VERTEX_INDEX_CONVERSION_OVERFLOWED);
    }

    if (property == "nx" || property == "ny" || property == "nz") {
      return MakeError(ErrorCode::NORMAL_CONVERSION_FAILED);
    }

    if (property == "texture_s" || property == "texture_t" ||
        property == "texture_u" || property == "texture_v" || property == "s" ||
        property == "t" || property == "u" || property == "v") {
      return MakeError(ErrorCode::TEXTURE_COORDINATE_CONVERSION_FAILED);
    }

    return std::error_code();
  }

  uintmax_t num_vertices_;
  size_t handle_vertex_index_;
  size_t current_vertex_index_;

  std::array<NormalType, 3> *normal_ = nullptr;
  std::array<UVType, 2> *uv_ = nullptr;

  std::array<PositionType, 3> xyz_ = {0.0, 0.0, 0.0};
  std::array<NormalType, 3> normal_storage_ = {0.0, 0.0, 0.0};
  std::array<UVType, 2> uv_storage_ = {0.0, 0.0};
};

template <std::floating_point PositionType, std::floating_point NormalType,
          std::floating_point UVType, std::unsigned_integral VertexIndexType>
  requires(sizeof(PositionType) <= sizeof(double) &&
           sizeof(NormalType) <= sizeof(double) &&
           sizeof(UVType) <= sizeof(double) &&
           sizeof(VertexIndexType) <= sizeof(uint32_t))
TriangleMeshReader<PositionType, NormalType, UVType,
                   VertexIndexType>::ErrorCategory
    TriangleMeshReader<PositionType, NormalType, UVType,
                       VertexIndexType>::error_category;

}  // namespace plyodine

#endif  // _PLYODINE_READERS_TRIANGLE_MESH_READER_
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
// For normals and texture coordinates, which are both optional,
// `TriangleMeshReader` will not emit either unless all of its coordinates are
// present (X, Y, and Z for normals and U and V for texture coordinates).
// If only a subset of the required coordinates are present, those coordinates
// will still be validated in isolation. Additionally, in the event that
// multiple aliases are present for the same texture coordinate only one alias
// will be selected for that coordinate and the will have their values validated
// then discarded. The exact ordering of the alias selection is not defined.
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
  requires(std::is_same_v<PositionType, float> ||
           std::is_same_v<PositionType, double>) &&
          (std::is_same_v<NormalType, float> ||
           std::is_same_v<NormalType, double>) &&
          (std::is_same_v<UVType, float> || std::is_same_v<UVType, double>) &&
          (std::is_same_v<VertexIndexType, uint8_t> ||
           std::is_same_v<VertexIndexType, uint16_t> ||
           std::is_same_v<VertexIndexType, uint32_t>)
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
  virtual void AddVertex(const std::array<PositionType, 3>& position,
                         const std::array<NormalType, 3>* maybe_normal,
                         const std::array<UVType, 2>* maybe_uv) = 0;

  // This function is implemented by derived classes in order to receive the
  // vertex indices of each triangle in the model.
  virtual void AddTriangle(
      const std::array<VertexIndexType, 3>& vertex_indices) = 0;

 private:
  enum class ErrorCode {
    MIN_VALUE = 1,
    MISSING_VERTEX_ELEMENT = 1,
    MISSING_FACE_ELEMENT = 2,
    MISSING_PROPERTY_X = 3,
    INVALID_PROPERTY_X_TYPE = 4,
    MISSING_PROPERTY_Y = 5,
    INVALID_PROPERTY_Y_TYPE = 6,
    MISSING_PROPERTY_Z = 7,
    INVALID_PROPERTY_Z_TYPE = 8,
    MISSING_PROPERTY_VERTEX_INDICES = 9,
    INVALID_PROPERTY_VERTEX_INDEX_TYPE = 10,
    INVALID_PROPERTY_NX_TYPE = 11,
    INVALID_PROPERTY_NY_TYPE = 12,
    INVALID_PROPERTY_NZ_TYPE = 13,
    INVALID_PROPERTY_TEXTURE_S_TYPE = 14,
    INVALID_PROPERTY_TEXTURE_T_TYPE = 15,
    INVALID_PROPERTY_TEXTURE_U_TYPE = 16,
    INVALID_PROPERTY_TEXTURE_V_TYPE = 17,
    INVALID_PROPERTY_S_TYPE = 18,
    INVALID_PROPERTY_T_TYPE = 19,
    INVALID_PROPERTY_U_TYPE = 20,
    INVALID_PROPERTY_V_TYPE = 21,
    INVALID_PROPERTY_X_VALUE = 22,
    INVALID_PROPERTY_Y_VALUE = 23,
    INVALID_PROPERTY_Z_VALUE = 24,
    INVALID_PROPERTY_VERTEX_INDEX_VALUE = 25,
    INVALID_PROPERTY_NX_VALUE = 26,
    INVALID_PROPERTY_NY_VALUE = 27,
    INVALID_PROPERTY_NZ_VALUE = 28,
    INVALID_PROPERTY_TEXTURE_S_VALUE = 29,
    INVALID_PROPERTY_TEXTURE_T_VALUE = 30,
    INVALID_PROPERTY_TEXTURE_U_VALUE = 31,
    INVALID_PROPERTY_TEXTURE_V_VALUE = 32,
    INVALID_PROPERTY_S_VALUE = 33,
    INVALID_PROPERTY_T_VALUE = 34,
    INVALID_PROPERTY_U_VALUE = 35,
    INVALID_PROPERTY_V_VALUE = 36,
    OVERFLOWED_PROPERTY_X_TYPE = 37,
    OVERFLOWED_PROPERTY_Y_TYPE = 38,
    OVERFLOWED_PROPERTY_Z_TYPE = 39,
    OVERFLOWED_PROPERTY_VERTEX_INDEX_TYPE = 40,
    OVERFLOWED_PROPERTY_NX_TYPE = 41,
    OVERFLOWED_PROPERTY_NY_TYPE = 42,
    OVERFLOWED_PROPERTY_NZ_TYPE = 43,
    OVERFLOWED_PROPERTY_TEXTURE_S_TYPE = 44,
    OVERFLOWED_PROPERTY_TEXTURE_T_TYPE = 45,
    OVERFLOWED_PROPERTY_TEXTURE_U_TYPE = 46,
    OVERFLOWED_PROPERTY_TEXTURE_V_TYPE = 47,
    OVERFLOWED_PROPERTY_S_TYPE = 48,
    OVERFLOWED_PROPERTY_T_TYPE = 49,
    OVERFLOWED_PROPERTY_U_TYPE = 50,
    OVERFLOWED_PROPERTY_V_TYPE = 51,
    MAX_VALUE = 51,
  };

  static class ErrorCategory final : public std::error_category {
    const char* name() const noexcept override {
      return "plyodine::TriangleMeshReader";
    }

    std::string message(int condition) const override {
      ErrorCode error_code{condition};
      switch (error_code) {
        case ErrorCode::MISSING_VERTEX_ELEMENT:
          return "The input did not contain required element 'vertex'";
        case ErrorCode::MISSING_FACE_ELEMENT:
          return "The input did not contain required element 'face'";
        case ErrorCode::MISSING_PROPERTY_X:
          return "The input did not contain required property 'x' on element "
                 "'vertex'";
        case ErrorCode::INVALID_PROPERTY_X_TYPE:
          return "The input specified an invalid type for property 'x' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::MISSING_PROPERTY_Y:
          return "The input did not contain required property 'y' on element "
                 "'vertex'";
        case ErrorCode::INVALID_PROPERTY_Y_TYPE:
          return "The input specified an invalid type for property 'y' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::MISSING_PROPERTY_Z:
          return "The input did not contain required property 'z' on element "
                 "'vertex'";
        case ErrorCode::INVALID_PROPERTY_Z_TYPE:
          return "The input specified an invalid type for property 'z' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::MISSING_PROPERTY_VERTEX_INDICES:
          return "The input did not contain required property 'vertex_indices' "
                 "on element 'face'";
        case ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_TYPE:
          return "The input specified an invalid type for property "
                 "'vertex_indices' on element 'face' (must be one of 'char', "
                 "'uchar', 'short', 'ushort', 'int', or 'uint')";
        case ErrorCode::INVALID_PROPERTY_NX_TYPE:
          return "The input specified an invalid type for property 'nx' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_NY_TYPE:
          return "The input specified an invalid type for property 'ny' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_NZ_TYPE:
          return "The input specified an invalid type for property 'nz' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_S_TYPE:
          return "The input specified an invalid type for property 'texture_s' "
                 "on element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_T_TYPE:
          return "The input specified an invalid type for property 'texture_t' "
                 "on element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_U_TYPE:
          return "The input specified an invalid type for property 'texture_u' "
                 "on element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_V_TYPE:
          return "The input specified an invalid type for property 'texture_v' "
                 "on element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_S_TYPE:
          return "The input specified an invalid type for property 's' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_T_TYPE:
          return "The input specified an invalid type for property 't' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_U_TYPE:
          return "The input specified an invalid type for property 'u' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_V_TYPE:
          return "The input specified an invalid type for property 'v' on "
                 "element 'vertex' (must be 'float' or 'double')";
        case ErrorCode::INVALID_PROPERTY_X_VALUE:
          return "The input contained an invalid value for property 'x' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_Y_VALUE:
          return "The input contained an invalid value for property 'y' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_Z_VALUE:
          return "The input contained an invalid value for property 'z' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_VALUE:
          return "The input contained an invalid entry of property list "
                 "'vertex_indices' on element 'face' (must be an index between "
                 "0 and the number of instances of element 'vertex')";
        case ErrorCode::INVALID_PROPERTY_NX_VALUE:
          return "The input contained an invalid value for property 'nx' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_NY_VALUE:
          return "The input contained an invalid value for property 'ny' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_NZ_VALUE:
          return "The input contained an invalid value for property 'nz' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_S_VALUE:
          return "The input contained an invalid value for property "
                 "'texture_s' on element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_T_VALUE:
          return "The input contained an invalid value for property "
                 "'texture_t' on element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_U_VALUE:
          return "The input contained an invalid value for property "
                 "'texture_u' on element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_TEXTURE_V_VALUE:
          return "The input contained an invalid value for property "
                 "'texture_v' on element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_S_VALUE:
          return "The input contained an invalid value for property 's' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_T_VALUE:
          return "The input contained an invalid value for property 't' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_U_VALUE:
          return "The input contained an invalid value for property 'u' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::INVALID_PROPERTY_V_VALUE:
          return "The input contained an invalid value for property 'v' on "
                 "element 'vertex' (must be finite)";
        case ErrorCode::OVERFLOWED_PROPERTY_X_TYPE:
          return "The input contained a value of property 'x' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_Y_TYPE:
          return "The input contained a value of property 'y' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_Z_TYPE:
          return "The input contained a value of property 'z' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_VERTEX_INDEX_TYPE:
          if constexpr (std::is_same_v<VertexIndexType, uint8_t>) {
            return "The input contained an entry of property list "
                   "'vertex_indices' on element 'face' that could not fit into "
                   "destination type 'uchar'";
          } else if constexpr (std::is_same_v<VertexIndexType, uint16_t>) {
            return "The input contained an entry of property list "
                   "'vertex_indices' on element 'face' that could not fit into "
                   "destination type 'ushort'";
          } else {
            static_assert(std::is_same_v<VertexIndexType, uint32_t>);
            return "The input contained an entry of property list "
                   "'vertex_indices' on element 'face' that could not fit into "
                   "destination type 'uint'";
          }
        case ErrorCode::OVERFLOWED_PROPERTY_NX_TYPE:
          return "The input contained a value of property 'nx' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_NY_TYPE:
          return "The input contained a value of property 'ny' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_NZ_TYPE:
          return "The input contained a value of property 'nz' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_S_TYPE:
          return "The input contained a value of property 'texture_s' on "
                 "element 'vertex' that could not fit finitely into "
                 "destination type 'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_T_TYPE:
          return "The input contained a value of property 'texture_t' on "
                 "element 'vertex' that could not fit finitely into "
                 "destination type 'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_U_TYPE:
          return "The input contained a value of property 'texture_u' on "
                 "element 'vertex' that could not fit finitely into "
                 "destination type 'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_V_TYPE:
          return "The input contained a value of property 'texture_v' on "
                 "element 'vertex' that could not fit finitely into "
                 "destination type 'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_S_TYPE:
          return "The input contained a value of property 's' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_T_TYPE:
          return "The input contained a value of property 't' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_U_TYPE:
          return "The input contained a value of property 'u' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
        case ErrorCode::OVERFLOWED_PROPERTY_V_TYPE:
          return "The input contained a value of property 'v' on element "
                 "'vertex' that could not fit finitely into destination type "
                 "'float'";
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

  static bool FloatingPointCallback(const PropertyCallback& callback) {
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
      std::map<std::string, PropertyCallback>& callbacks, size_t index) {
    static const std::string property_names[3] = {"x", "y", "z"};
    static const ErrorCode missing_error_codes[3] = {
        ErrorCode::MISSING_PROPERTY_X, ErrorCode::MISSING_PROPERTY_Y,
        ErrorCode::MISSING_PROPERTY_Z};
    static const ErrorCode invalid_type_error_codes[3] = {
        ErrorCode::INVALID_PROPERTY_X_TYPE, ErrorCode::INVALID_PROPERTY_Y_TYPE,
        ErrorCode::INVALID_PROPERTY_Z_TYPE};
    static const ErrorCode invalid_value_error_codes[3] = {
        ErrorCode::INVALID_PROPERTY_X_VALUE,
        ErrorCode::INVALID_PROPERTY_Y_VALUE,
        ErrorCode::INVALID_PROPERTY_Z_VALUE};

    auto iter = callbacks.find(property_names[index]);
    if (iter == callbacks.end()) {
      return MakeError(missing_error_codes[index]);
    }

    if (!FloatingPointCallback(iter->second)) {
      return MakeError(invalid_type_error_codes[index]);
    }

    iter->second = std::function<std::error_code(PositionType)>(
        [index, this](PositionType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(invalid_value_error_codes[index]);
          }

          xyz_[index] = value;

          return MaybeAddVertex();
        });

    return std::error_code();
  }

  std::error_code AddVertexIndicesCallback(
      std::map<std::string, PropertyCallback>& callbacks,
      uintmax_t num_vertices) {
    auto iter = callbacks.find("vertex_indices");
    if (iter == callbacks.end()) {
      return MakeError(ErrorCode::MISSING_PROPERTY_VERTEX_INDICES);
    }

    if (!std::holds_alternative<CharPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UCharPropertyListCallback>(iter->second) &&
        !std::holds_alternative<ShortPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UShortPropertyListCallback>(iter->second) &&
        !std::holds_alternative<IntPropertyListCallback>(iter->second) &&
        !std::holds_alternative<UIntPropertyListCallback>(iter->second)) {
      return MakeError(ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_TYPE);
    }

    iter->second =
        std::function<std::error_code(std::span<const VertexIndexType>)>(
            [num_vertices, this](std::span<const VertexIndexType> indices) {
              if (indices.size() < 3) {
                return std::error_code();
              }

              if (num_vertices <= indices[0] || num_vertices <= indices[1]) {
                return MakeError(
                    ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_VALUE);
              }

              std::array<VertexIndexType, 3> faces;
              faces[0] = static_cast<VertexIndexType>(indices[0]);
              for (size_t i = 2u; i < indices.size(); i++) {
                if (num_vertices <= indices[i]) {
                  return MakeError(
                      ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_VALUE);
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

  void AddVertexNormalCallback(PropertyCallback& callbacks, size_t index) {
    static const ErrorCode invalid_value_error_codes[3] = {
        ErrorCode::INVALID_PROPERTY_NX_VALUE,
        ErrorCode::INVALID_PROPERTY_NY_VALUE,
        ErrorCode::INVALID_PROPERTY_NZ_VALUE};

    callbacks = std::function<std::error_code(NormalType)>(
        [index, this](NormalType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(invalid_value_error_codes[index]);
          }

          if (normal_storage_.size() <= index) {
            return std::error_code();
          }

          normal_storage_[index] = value;

          return MaybeAddVertex();
        });
  }

  std::error_code AddVertexNormalCallbacks(
      std::map<std::string, PropertyCallback>& callbacks) {
    auto x_iter = callbacks.find("nx");
    auto y_iter = callbacks.find("ny");
    auto z_iter = callbacks.find("nz");

    bool success = true;
    if (x_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(x_iter->second)) {
      return MakeError(ErrorCode::INVALID_PROPERTY_NX_TYPE);
    } else {
      AddVertexNormalCallback(x_iter->second, 3);
    }

    if (y_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(y_iter->second)) {
      return MakeError(ErrorCode::INVALID_PROPERTY_NY_TYPE);
    } else {
      AddVertexNormalCallback(y_iter->second, 3);
    }

    if (z_iter == callbacks.end()) {
      success = false;
    } else if (!FloatingPointCallback(z_iter->second)) {
      return MakeError(ErrorCode::INVALID_PROPERTY_NZ_TYPE);
    } else {
      AddVertexNormalCallback(z_iter->second, 3);
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

  void AddVertexUVCallback(PropertyCallback& callbacks, ErrorCode error_code,
                           size_t index) {
    callbacks = std::function<std::error_code(UVType)>(
        [error_code, index, this](UVType value) -> std::error_code {
          if (!std::isfinite(value)) {
            return MakeError(error_code);
          }

          if (uv_storage_.size() <= index) {
            return std::error_code();
          }

          uv_storage_[index] = value;

          return MaybeAddVertex();
        });
  }

  std::error_code AddVertexUVCallbacks(
      std::map<std::string, PropertyCallback>& callbacks) {
    auto texture_s_iter = callbacks.find("texture_s");
    auto texture_t_iter = callbacks.find("texture_t");
    auto texture_u_iter = callbacks.find("texture_u");
    auto texture_v_iter = callbacks.find("texture_v");
    auto s_iter = callbacks.find("s");
    auto t_iter = callbacks.find("t");
    auto u_iter = callbacks.find("u");
    auto v_iter = callbacks.find("v");

    ErrorCode selected_u_error;
    auto selected_u_iter = callbacks.end();
    if (texture_s_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_s_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_TEXTURE_S_TYPE);
      }

      selected_u_iter = texture_s_iter;
      selected_u_error = ErrorCode::INVALID_PROPERTY_TEXTURE_S_VALUE;

      AddVertexUVCallback(selected_u_iter->second, selected_u_error, 2);
    }

    if (texture_u_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_u_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_TEXTURE_U_TYPE);
      }

      selected_u_iter = texture_u_iter;
      selected_u_error = ErrorCode::INVALID_PROPERTY_TEXTURE_U_VALUE;

      AddVertexUVCallback(selected_u_iter->second, selected_u_error, 2);
    }

    if (s_iter != callbacks.end()) {
      if (!FloatingPointCallback(s_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_S_TYPE);
      }

      selected_u_iter = s_iter;
      selected_u_error = ErrorCode::INVALID_PROPERTY_S_VALUE;

      AddVertexUVCallback(selected_u_iter->second, selected_u_error, 2);
    }

    if (u_iter != callbacks.end()) {
      if (!FloatingPointCallback(u_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_U_TYPE);
      }

      selected_u_iter = u_iter;
      selected_u_error = ErrorCode::INVALID_PROPERTY_U_VALUE;

      AddVertexUVCallback(selected_u_iter->second, selected_u_error, 2);
    }

    ErrorCode selected_v_error;
    auto selected_v_iter = callbacks.end();
    if (texture_t_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_t_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_TEXTURE_T_TYPE);
      }

      selected_v_iter = texture_t_iter;
      selected_v_error = ErrorCode::INVALID_PROPERTY_TEXTURE_T_VALUE;

      AddVertexUVCallback(selected_v_iter->second, selected_v_error, 2);
    }

    if (texture_v_iter != callbacks.end()) {
      if (!FloatingPointCallback(texture_v_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_TEXTURE_V_TYPE);
      }

      selected_v_iter = texture_v_iter;
      selected_v_error = ErrorCode::INVALID_PROPERTY_TEXTURE_V_VALUE;

      AddVertexUVCallback(selected_v_iter->second, selected_v_error, 2);
    }

    if (t_iter != callbacks.end()) {
      if (!FloatingPointCallback(t_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_T_TYPE);
      }

      selected_v_iter = t_iter;
      selected_v_error = ErrorCode::INVALID_PROPERTY_T_VALUE;

      AddVertexUVCallback(selected_v_iter->second, selected_v_error, 2);
    }

    if (v_iter != callbacks.end()) {
      if (!FloatingPointCallback(v_iter->second)) {
        return MakeError(ErrorCode::INVALID_PROPERTY_V_TYPE);
      }

      selected_v_iter = v_iter;
      selected_v_error = ErrorCode::INVALID_PROPERTY_V_VALUE;

      AddVertexUVCallback(selected_v_iter->second, selected_v_error, 2);
    }

    if (selected_u_iter != callbacks.end() &&
        selected_v_iter != callbacks.end()) {
      AddVertexUVCallback(selected_u_iter->second, selected_u_error, 0);
      AddVertexUVCallback(selected_v_iter->second, selected_v_error, 1);

      uv_ = &uv_storage_;
      handle_vertex_index_ += 2;
    }

    return std::error_code();
  }

  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
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

  std::error_code OnConversionFailure(const std::string& element,
                                      const std::string& property,
                                      ConversionFailureReason reason) override {
    if (property == "x") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_X_TYPE);
    } else if (property == "y") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_Y_TYPE);
    } else if (property == "z") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_Z_TYPE);
    } else if (property == "vertex_indices") {
      if (reason == ConversionFailureReason::INTEGER_OVERFLOW) {
        return MakeError(ErrorCode::OVERFLOWED_PROPERTY_VERTEX_INDEX_TYPE);
      }
      return MakeError(ErrorCode::INVALID_PROPERTY_VERTEX_INDEX_VALUE);
    } else if (property == "nx") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_NX_TYPE);
    } else if (property == "ny") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_NY_TYPE);
    } else if (property == "nz") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_NZ_TYPE);
    } else if (property == "texture_s") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_S_TYPE);
    } else if (property == "texture_t") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_T_TYPE);
    } else if (property == "texture_u") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_U_TYPE);
    } else if (property == "texture_v") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_TEXTURE_V_TYPE);
    } else if (property == "s") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_S_TYPE);
    } else if (property == "t") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_T_TYPE);
    } else if (property == "u") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_U_TYPE);
    } else if (property == "v") {
      return MakeError(ErrorCode::OVERFLOWED_PROPERTY_V_TYPE);
    }
    return std::error_code();
  }

  uintmax_t num_vertices_;
  size_t handle_vertex_index_;
  size_t current_vertex_index_;

  std::array<NormalType, 3>* normal_ = nullptr;
  std::array<UVType, 2>* uv_ = nullptr;

  std::array<PositionType, 3> xyz_ = {0.0, 0.0, 0.0};
  std::array<NormalType, 3> normal_storage_ = {0.0, 0.0, 0.0};
  std::array<UVType, 2> uv_storage_ = {0.0, 0.0};
};

template <std::floating_point PositionType, std::floating_point NormalType,
          std::floating_point UVType, std::unsigned_integral VertexIndexType>
  requires(std::is_same_v<PositionType, float> ||
           std::is_same_v<PositionType, double>) &&
          (std::is_same_v<NormalType, float> ||
           std::is_same_v<NormalType, double>) &&
          (std::is_same_v<UVType, float> || std::is_same_v<UVType, double>) &&
          (std::is_same_v<VertexIndexType, uint8_t> ||
           std::is_same_v<VertexIndexType, uint16_t> ||
           std::is_same_v<VertexIndexType, uint32_t>)
TriangleMeshReader<PositionType, NormalType, UVType,
                   VertexIndexType>::ErrorCategory
    TriangleMeshReader<PositionType, NormalType, UVType,
                       VertexIndexType>::error_category;

}  // namespace plyodine

#endif  // _PLYODINE_READERS_TRIANGLE_MESH_READER_
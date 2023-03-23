#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "plyodine/ply_property.h"

namespace plyodine {

class PlyReader {
 public:
  std::expected<void, std::string_view> ReadFrom(std::istream& stream);

  virtual std::expected<void, std::string_view> Start(
      const std::unordered_map<
          std::string_view,
          std::unordered_map<std::string_view,
                             std::pair<size_t, Property::Type>>>& properties,
      std::span<const std::string> comments) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int8Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int8PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt8Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt8PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int16Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int16PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt16Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt16PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int32Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int32PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt32Property value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt32PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, FloatProperty value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, FloatPropertyList values) = 0;

  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, DoubleProperty value) = 0;
  virtual std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, DoublePropertyList values) = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
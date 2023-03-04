#ifndef _PLYODINE_STREAMING_PLY_READER_
#define _PLYODINE_STREAMING_PLY_READER_

#include <expected>
#include <map>
#include <span>
#include <string>
#include <string_view>

#include "plyodine/property.h"

namespace plyodine {

class Reader {
 public:
  virtual std::expected<void, std::string_view> Start(
      const std::map<std::string,
                     std::map<std::string, std::pair<size_t, Property::Type>>>&
          properties) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int8Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int8PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt8Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt8PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int16Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int16PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt16Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt16PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int32Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, Int32PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt32Property value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, UInt32PropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, FloatProperty value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, FloatPropertyList values) = 0;

  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, DoubleProperty value) = 0;
  virtual std::expected<void, std::string_view> Parse(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, DoublePropertyList values) = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_STREAMING_PLY_READER_
#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <expected>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include "plyodine/ply_property.h"

namespace plyodine {

class PlyWriter {
 public:
  // Writes a binary encoded output matching the system native endianness
  std::expected<void, std::string_view> WriteTo(std::ostream& stream);

  // Most clients should prefer WriteTo over these
  std::expected<void, std::string_view> WriteToASCII(std::ostream& stream);
  std::expected<void, std::string_view> WriteToBigEndian(std::ostream& stream);
  std::expected<void, std::string_view> WriteToLittleEndian(
      std::ostream& stream);

  typedef std::expected<Int8Property, std::string_view> (
      PlyWriter::*Int8PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<Int8PropertyList, std::string_view> (
      PlyWriter::*Int8PropertyListCallback)(std::string_view, std::string_view);
  typedef std::expected<UInt8Property, std::string_view> (
      PlyWriter::*UInt8PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<UInt8PropertyList, std::string_view> (
      PlyWriter::*UInt8PropertyListCallback)(std::string_view,
                                             std::string_view);
  typedef std::expected<Int16Property, std::string_view> (
      PlyWriter::*Int16PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<Int16PropertyList, std::string_view> (
      PlyWriter::*Int16PropertyListCallback)(std::string_view,
                                             std::string_view);
  typedef std::expected<UInt16Property, std::string_view> (
      PlyWriter::*UInt16PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<UInt16PropertyList, std::string_view> (
      PlyWriter::*UInt16PropertyListCallback)(std::string_view,
                                              std::string_view);
  typedef std::expected<Int32Property, std::string_view> (
      PlyWriter::*Int32PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<Int32PropertyList, std::string_view> (
      PlyWriter::*Int32PropertyListCallback)(std::string_view,
                                             std::string_view);
  typedef std::expected<UInt32Property, std::string_view> (
      PlyWriter::*UInt32PropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<UInt32PropertyList, std::string_view> (
      PlyWriter::*UInt32PropertyListCallback)(std::string_view,
                                              std::string_view);
  typedef std::expected<FloatProperty, std::string_view> (
      PlyWriter::*FloatPropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<FloatPropertyList, std::string_view> (
      PlyWriter::*FloatPropertyListCallback)(std::string_view,
                                             std::string_view);
  typedef std::expected<DoubleProperty, std::string_view> (
      PlyWriter::*DoublePropertyCallback)(std::string_view, std::string_view);
  typedef std::expected<DoublePropertyList, std::string_view> (
      PlyWriter::*DoublePropertyListCallback)(std::string_view,
                                              std::string_view);

  typedef std::variant<Int8PropertyCallback, Int8PropertyListCallback,
                       UInt8PropertyCallback, UInt8PropertyListCallback,
                       Int16PropertyCallback, Int16PropertyListCallback,
                       UInt16PropertyCallback, UInt16PropertyListCallback,
                       Int32PropertyCallback, Int32PropertyListCallback,
                       UInt32PropertyCallback, UInt32PropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      Callback;

  virtual std::expected<void, std::string_view> Start(
      std::map<std::string_view,
               std::pair<uint64_t, std::map<std::string_view, Callback>>>&
          property_callbacks,
      std::span<const std::string>& comments,
      std::span<const std::string>& object_info) = 0;

  enum SizeType {
    UINT8 = static_cast<int>(PropertyType::UINT8),
    UINT16 = static_cast<int>(PropertyType::UINT16),
    UINT32 = static_cast<int>(PropertyType::UINT32),
  };

  virtual std::expected<SizeType, std::string_view> GetPropertyListSizeType(
      std::string_view element_name, std::string_view property_name) const = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
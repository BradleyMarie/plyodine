#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include "plyodine/ply_property.h"

namespace plyodine {

class PlyReader {
 public:
  std::expected<void, std::string_view> ReadFrom(std::istream& stream);

  typedef std::expected<void, std::string_view> (
      PlyReader::*Int8PropertyCallback)(std::string_view, std::string_view,
                                        Int8Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*Int8PropertyListCallback)(std::string_view, std::string_view,
                                            Int8PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt8PropertyCallback)(std::string_view, std::string_view,
                                         UInt8Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt8PropertyListCallback)(std::string_view, std::string_view,
                                             UInt8PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*Int16PropertyCallback)(std::string_view, std::string_view,
                                         Int16Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*Int16PropertyListCallback)(std::string_view, std::string_view,
                                             Int16PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt16PropertyCallback)(std::string_view, std::string_view,
                                          UInt16Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt16PropertyListCallback)(std::string_view,
                                              std::string_view,
                                              UInt16PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*Int32PropertyCallback)(std::string_view, std::string_view,
                                         Int32Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*Int32PropertyListCallback)(std::string_view, std::string_view,
                                             Int32PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt32PropertyCallback)(std::string_view, std::string_view,
                                          UInt32Property);
  typedef std::expected<void, std::string_view> (
      PlyReader::*UInt32PropertyListCallback)(std::string_view,
                                              std::string_view,
                                              UInt32PropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*FloatPropertyCallback)(std::string_view, std::string_view,
                                         FloatProperty);
  typedef std::expected<void, std::string_view> (
      PlyReader::*FloatPropertyListCallback)(std::string_view, std::string_view,
                                             FloatPropertyList);
  typedef std::expected<void, std::string_view> (
      PlyReader::*DoublePropertyCallback)(std::string_view, std::string_view,
                                          DoubleProperty);
  typedef std::expected<void, std::string_view> (
      PlyReader::*DoublePropertyListCallback)(std::string_view,
                                              std::string_view,
                                              DoublePropertyList);

  typedef std::variant<Int8PropertyCallback, Int8PropertyListCallback,
                       UInt8PropertyCallback, UInt8PropertyListCallback,
                       Int16PropertyCallback, Int16PropertyListCallback,
                       UInt16PropertyCallback, UInt16PropertyListCallback,
                       Int32PropertyCallback, Int32PropertyListCallback,
                       UInt32PropertyCallback, UInt32PropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      Callback;

  virtual std::expected<
      std::map<std::string_view, std::map<std::string_view, Callback>>,
      std::string_view>
  Start(const std::map<
            std::string_view,
            std::pair<uint64_t, std::map<std::string_view, Property::Type>>>&
            properties,
        std::span<const std::string> comments,
        std::span<const std::string> object_info) = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
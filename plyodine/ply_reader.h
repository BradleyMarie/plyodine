#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "plyodine/ply_property.h"

namespace plyodine {

class PlyReader {
 public:
  std::expected<void, std::string> ReadFrom(std::istream& stream);

 protected:
  typedef std::expected<void, std::string> (PlyReader::*Int8PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      Int8Property);
  typedef std::expected<void, std::string> (
      PlyReader::*Int8PropertyListCallback)(const std::string&, size_t,
                                            const std::string&, size_t,
                                            uint64_t, Int8PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*UInt8PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      UInt8Property);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt8PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t, UInt8PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*Int16PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      Int16Property);
  typedef std::expected<void, std::string> (
      PlyReader::*Int16PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t, Int16PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*UInt16PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      UInt16Property);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt16PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uint64_t, UInt16PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*Int32PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      Int32Property);
  typedef std::expected<void, std::string> (
      PlyReader::*Int32PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t, Int32PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*UInt32PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      UInt32Property);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt32PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uint64_t, UInt32PropertyList);
  typedef std::expected<void, std::string> (PlyReader::*FloatPropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      FloatProperty);
  typedef std::expected<void, std::string> (
      PlyReader::*FloatPropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t, FloatPropertyList);
  typedef std::expected<void, std::string> (PlyReader::*DoublePropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      DoubleProperty);
  typedef std::expected<void, std::string> (
      PlyReader::*DoublePropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uint64_t, DoublePropertyList);

  typedef std::variant<Int8PropertyCallback, Int8PropertyListCallback,
                       UInt8PropertyCallback, UInt8PropertyListCallback,
                       Int16PropertyCallback, Int16PropertyListCallback,
                       UInt16PropertyCallback, UInt16PropertyListCallback,
                       Int32PropertyCallback, Int32PropertyListCallback,
                       UInt32PropertyCallback, UInt32PropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      Callback;

  virtual std::expected<std::map<std::string, std::map<std::string, Callback>>,
                        std::string>
  Start(
      const std::map<std::string,
                     std::pair<uint64_t, std::map<std::string, PropertyType>>>&
          properties,
      const std::vector<std::string>& comments,
      const std::vector<std::string>& object_info) = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
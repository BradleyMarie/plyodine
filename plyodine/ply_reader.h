#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <cstdint>
#include <expected>
#include <istream>
#include <map>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace plyodine {

class PlyReader {
 public:
  // NOTE: Behavior is undefined if stream is not a binary stream
  std::expected<void, std::string> ReadFrom(std::istream& stream);

 protected:
  typedef std::expected<void, std::string> (PlyReader::*Int8PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      int8_t);
  typedef std::expected<void, std::string> (
      PlyReader::*Int8PropertyListCallback)(const std::string&, size_t,
                                            const std::string&, size_t,
                                            uintmax_t, std::span<const int8_t>);

  typedef std::expected<void, std::string> (PlyReader::*UInt8PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      uint8_t);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt8PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::span<const uint8_t>);

  typedef std::expected<void, std::string> (PlyReader::*Int16PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      int16_t);
  typedef std::expected<void, std::string> (
      PlyReader::*Int16PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::span<const int16_t>);

  typedef std::expected<void, std::string> (PlyReader::*UInt16PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      uint16_t);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt16PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::span<const uint16_t>);

  typedef std::expected<void, std::string> (PlyReader::*Int32PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      int32_t);
  typedef std::expected<void, std::string> (
      PlyReader::*Int32PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::span<const int32_t>);

  typedef std::expected<void, std::string> (PlyReader::*UInt32PropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      uint32_t);
  typedef std::expected<void, std::string> (
      PlyReader::*UInt32PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::span<const uint32_t>);

  typedef std::expected<void, std::string> (PlyReader::*FloatPropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t, float);
  typedef std::expected<void, std::string> (
      PlyReader::*FloatPropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t, std::span<const float>);

  typedef std::expected<void, std::string> (PlyReader::*DoublePropertyCallback)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t,
      double);
  typedef std::expected<void, std::string> (
      PlyReader::*DoublePropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::span<const double>);

  typedef std::variant<Int8PropertyCallback, Int8PropertyListCallback,
                       UInt8PropertyCallback, UInt8PropertyListCallback,
                       Int16PropertyCallback, Int16PropertyListCallback,
                       UInt16PropertyCallback, UInt16PropertyListCallback,
                       Int32PropertyCallback, Int32PropertyListCallback,
                       UInt32PropertyCallback, UInt32PropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      Callback;

  virtual std::expected<void, std::string> Start(
      const std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, Callback>>& callbacks,
      const std::vector<std::string>& comments,
      const std::vector<std::string>& object_info) = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
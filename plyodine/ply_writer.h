#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <cstdint>
#include <expected>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace plyodine {

class PlyWriter {
 public:
  // Writes a binary encoded output matching the system native endianness
  // NOTE: Behavior is undefined if stream is not a binary stream
  std::error_code WriteTo(std::ostream& stream) const;

  // Most clients should prefer WriteTo over these
  // NOTE: Behavior is undefined if stream is not a binary stream
  std::error_code WriteToASCII(std::ostream& stream) const;
  std::error_code WriteToBigEndian(std::ostream& stream) const;
  std::error_code WriteToLittleEndian(std::ostream& stream) const;

 protected:
  typedef std::expected<int8_t, std::error_code> (
      PlyWriter::*Int8PropertyCallback)(const std::string&, size_t,
                                        const std::string&, size_t,
                                        uintmax_t) const;
  typedef std::expected<std::span<const int8_t>, std::error_code> (
      PlyWriter::*Int8PropertyListCallback)(const std::string&, size_t,
                                            const std::string&, size_t,
                                            uintmax_t,
                                            std::vector<int8_t>&) const;

  typedef std::expected<uint8_t, std::error_code> (
      PlyWriter::*UInt8PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uintmax_t) const;
  typedef std::expected<std::span<const uint8_t>, std::error_code> (
      PlyWriter::*UInt8PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::vector<uint8_t>&) const;

  typedef std::expected<int16_t, std::error_code> (
      PlyWriter::*Int16PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uintmax_t) const;
  typedef std::expected<std::span<const int16_t>, std::error_code> (
      PlyWriter::*Int16PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::vector<int16_t>&) const;

  typedef std::expected<uint16_t, std::error_code> (
      PlyWriter::*UInt16PropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uintmax_t) const;
  typedef std::expected<std::span<const uint16_t>, std::error_code> (
      PlyWriter::*UInt16PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::vector<uint16_t>&) const;

  typedef std::expected<int32_t, std::error_code> (
      PlyWriter::*Int32PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uintmax_t) const;
  typedef std::expected<std::span<const int32_t>, std::error_code> (
      PlyWriter::*Int32PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::vector<int32_t>&) const;

  typedef std::expected<uint32_t, std::error_code> (
      PlyWriter::*UInt32PropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uintmax_t) const;
  typedef std::expected<std::span<const uint32_t>, std::error_code> (
      PlyWriter::*UInt32PropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::vector<uint32_t>&) const;

  typedef std::expected<float, std::error_code> (
      PlyWriter::*FloatPropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uintmax_t) const;
  typedef std::expected<std::span<const float>, std::error_code> (
      PlyWriter::*FloatPropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uintmax_t,
                                             std::vector<float>&) const;

  typedef std::expected<double, std::error_code> (
      PlyWriter::*DoublePropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uintmax_t) const;
  typedef std::expected<std::span<const double>, std::error_code> (
      PlyWriter::*DoublePropertyListCallback)(const std::string&, size_t,
                                              const std::string&, size_t,
                                              uintmax_t,
                                              std::vector<double>&) const;

  typedef std::variant<Int8PropertyCallback, Int8PropertyListCallback,
                       UInt8PropertyCallback, UInt8PropertyListCallback,
                       Int16PropertyCallback, Int16PropertyListCallback,
                       UInt16PropertyCallback, UInt16PropertyListCallback,
                       Int32PropertyCallback, Int32PropertyListCallback,
                       UInt32PropertyCallback, UInt32PropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      Callback;

  virtual std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, Callback>>& callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const = 0;

  enum class ListSizeType {
    UINT8 = 0,
    UINT16 = 1,
    UINT32 = 2,
  };

  virtual std::expected<PlyWriter::ListSizeType, std::error_code>
  GetPropertyListSizeType(const std::string& element_name, size_t element_index,
                          const std::string& property_name,
                          size_t property_index) const = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
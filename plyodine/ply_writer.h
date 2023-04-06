#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <expected>
#include <map>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

#include "plyodine/ply_property.h"

namespace plyodine {

class PlyWriter {
 public:
  // Writes a binary encoded output matching the system native endianness
  std::expected<void, std::string> WriteTo(std::ostream& stream) const;

  // Most clients should prefer WriteTo over these
  std::expected<void, std::string> WriteToASCII(std::ostream& stream) const;
  std::expected<void, std::string> WriteToBigEndian(std::ostream& stream) const;
  std::expected<void, std::string> WriteToLittleEndian(
      std::ostream& stream) const;

 protected:
  typedef std::expected<Int8Property, std::string> (
      PlyWriter::*Int8PropertyCallback)(const std::string&, size_t,
                                        const std::string&, size_t,
                                        uint64_t) const;
  typedef std::expected<Int8PropertyList, std::string> (
      PlyWriter::*Int8PropertyListCallback)(const std::string&, size_t,
                                            const std::string&, size_t,
                                            uint64_t,
                                            std::vector<Int8Property>&) const;
  typedef std::expected<UInt8Property, std::string> (
      PlyWriter::*UInt8PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uint64_t) const;
  typedef std::expected<UInt8PropertyList, std::string> (
      PlyWriter::*UInt8PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t,
                                             std::vector<UInt8Property>&) const;
  typedef std::expected<Int16Property, std::string> (
      PlyWriter::*Int16PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uint64_t) const;
  typedef std::expected<Int16PropertyList, std::string> (
      PlyWriter::*Int16PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t,
                                             std::vector<Int16Property>&) const;
  typedef std::expected<UInt16Property, std::string> (
      PlyWriter::*UInt16PropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uint64_t) const;
  typedef std::expected<UInt16PropertyList, std::string> (
      PlyWriter::*UInt16PropertyListCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      std::vector<UInt16Property>&) const;
  typedef std::expected<Int32Property, std::string> (
      PlyWriter::*Int32PropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uint64_t) const;
  typedef std::expected<Int32PropertyList, std::string> (
      PlyWriter::*Int32PropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t,
                                             std::vector<Int32Property>&) const;
  typedef std::expected<UInt32Property, std::string> (
      PlyWriter::*UInt32PropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uint64_t) const;
  typedef std::expected<UInt32PropertyList, std::string> (
      PlyWriter::*UInt32PropertyListCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      std::vector<UInt32Property>&) const;
  typedef std::expected<FloatProperty, std::string> (
      PlyWriter::*FloatPropertyCallback)(const std::string&, size_t,
                                         const std::string&, size_t,
                                         uint64_t) const;
  typedef std::expected<FloatPropertyList, std::string> (
      PlyWriter::*FloatPropertyListCallback)(const std::string&, size_t,
                                             const std::string&, size_t,
                                             uint64_t,
                                             std::vector<FloatProperty>&) const;
  typedef std::expected<DoubleProperty, std::string> (
      PlyWriter::*DoublePropertyCallback)(const std::string&, size_t,
                                          const std::string&, size_t,
                                          uint64_t) const;
  typedef std::expected<DoublePropertyList, std::string> (
      PlyWriter::*DoublePropertyListCallback)(
      const std::string&, size_t, const std::string&, size_t, uint64_t,
      std::vector<DoubleProperty>&) const;

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
      std::map<std::string,
               std::pair<uint64_t, std::map<std::string, Callback>>>&
          property_callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const = 0;

  enum class ListSizeType {
    UINT8 = static_cast<int>(PropertyType::UINT8),
    UINT16 = static_cast<int>(PropertyType::UINT16),
    UINT32 = static_cast<int>(PropertyType::UINT32),
  };

  virtual std::expected<PlyWriter::ListSizeType, std::string>
  GetPropertyListSizeType(const std::string& element_name, size_t element_index,
                          const std::string& property_name,
                          size_t property_index) const = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
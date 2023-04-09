#ifndef _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#define _PLYODINE_WRITERS_IN_MEMORY_WRITER_

#include <expected>
#include <map>
#include <ostream>
#include <string>
#include <variant>

#include "plyodine/ply_property.h"

namespace plyodine {

struct Property final
    : public std::variant<
          std::span<const Int8Property>, std::span<const Int8PropertyList>,
          std::span<const UInt8Property>, std::span<const UInt8PropertyList>,
          std::span<const Int16Property>, std::span<const Int16PropertyList>,
          std::span<const UInt16Property>, std::span<const UInt16PropertyList>,
          std::span<const Int32Property>, std::span<const Int32PropertyList>,
          std::span<const UInt32Property>, std::span<const UInt32PropertyList>,
          std::span<const FloatProperty>, std::span<const FloatPropertyList>,
          std::span<const DoubleProperty>,
          std::span<const DoublePropertyList>> {
  using std::variant<
      std::span<const Int8Property>, std::span<const Int8PropertyList>,
      std::span<const UInt8Property>, std::span<const UInt8PropertyList>,
      std::span<const Int16Property>, std::span<const Int16PropertyList>,
      std::span<const UInt16Property>, std::span<const UInt16PropertyList>,
      std::span<const Int32Property>, std::span<const Int32PropertyList>,
      std::span<const UInt32Property>, std::span<const UInt32PropertyList>,
      std::span<const FloatProperty>, std::span<const FloatPropertyList>,
      std::span<const DoubleProperty>,
      std::span<const DoublePropertyList>>::variant;

  size_t size() const {
    return std::visit([](const auto& entry) { return entry.size(); }, *this);
  }
};

std::expected<void, std::string> WriteTo(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

}  // namespace plyodine

#endif  // _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#ifndef _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#define _PLYODINE_WRITERS_IN_MEMORY_WRITER_

#include <cstdint>
#include <expected>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <variant>

namespace plyodine {

struct Property final
    : public std::variant<
          std::span<const int8_t>, std::span<const std::span<const int8_t>>,
          std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
          std::span<const int16_t>, std::span<const std::span<const int16_t>>,
          std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
          std::span<const int32_t>, std::span<const std::span<const int32_t>>,
          std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
          std::span<const float>, std::span<const std::span<const float>>,
          std::span<const double>, std::span<const std::span<const double>>> {
  using std::variant<
      std::span<const int8_t>, std::span<const std::span<const int8_t>>,
      std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
      std::span<const int16_t>, std::span<const std::span<const int16_t>>,
      std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
      std::span<const int32_t>, std::span<const std::span<const int32_t>>,
      std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
      std::span<const float>, std::span<const std::span<const float>>,
      std::span<const double>,
      std::span<const std::span<const double>>>::variant;

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
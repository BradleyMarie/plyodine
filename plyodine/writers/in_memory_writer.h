#ifndef _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#define _PLYODINE_WRITERS_IN_MEMORY_WRITER_

#include <expected>
#include <map>
#include <ostream>
#include <string>

#include "plyodine/ply_property.h"

namespace plyodine {

std::expected<void, std::string_view> WriteTo(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {});

}  // namespace plyodine

#endif  // _PLYODINE_WRITERS_IN_MEMORY_WRITER_
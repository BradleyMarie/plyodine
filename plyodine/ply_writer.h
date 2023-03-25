#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <expected>
#include <map>
#include <ostream>
#include <string_view>

#include "plyodine/ply_property.h"

namespace plyodine {

std::expected<void, std::string_view> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments = {},
    std::span<const std::string_view> object_info = {});

std::expected<void, std::string_view> WriteToBinary(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments = {},
    std::span<const std::string_view> object_info = {});

// NOTE: Most clients should not use this and should instead use WriteToBinary
std::expected<void, std::string_view> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments = {},
    std::span<const std::string_view> object_info = {});

// NOTE: Most clients should not use this and should instead use WriteToBinary
std::expected<void, std::string_view> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments = {},
    std::span<const std::string_view> object_info = {});

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
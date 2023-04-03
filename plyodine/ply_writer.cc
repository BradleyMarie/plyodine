#include "plyodine/ply_writer.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <type_traits>
#include <vector>

namespace plyodine {
namespace {

std::string_view FloatingPointError() {
  return "Only finite floating point values may be serialized to an ASCII "
         "output";
}

std::string_view WriteFailure() { return "Write failure"; }

std::expected<void, std::string_view> ValidateName(std::string_view name) {
  if (name.empty()) {
    return std::unexpected("Names of properties and elements may not be empty");
  }

  for (char c : name) {
    if (!std::isgraph(c)) {
      return std::unexpected(
          "Names of properties and elements may only contain graphic "
          "characters");
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ValidateProperties(
    const std::map<
        std::string_view,
        std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>&
        properties) {
  for (const auto& [element_name, element_properties] : properties) {
    auto element_name_valid = ValidateName(element_name);
    if (!element_name_valid) {
      return element_name_valid;
    }

    std::optional<size_t> property_size;
    for (const auto& [property_name, property] : element_properties.second) {
      auto property_name_valid = ValidateName(property_name);
      if (!property_name_valid) {
        return property_name_valid;
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ValidateComments(
    std::span<const std::string> comments) {
  for (const auto& comment : comments) {
    for (char c : comment) {
      if (c == '\r' || c == '\n') {
        return std::unexpected(
            "A comment may not contain line feed or carriage return");
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<std::vector<std::tuple<
                  std::string_view, uint64_t,
                  std::vector<std::tuple<std::string_view, PlyWriter::Callback,
                                         std::optional<PlyWriter::SizeType>>>>>,
              std::string_view>
GetListSizes(
    const PlyWriter& ply_writer,
    const std::map<
        std::string_view,
        std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>&
        properties) {
  std::vector<
      std::tuple<std::string_view, uint64_t,
                 std::vector<std::tuple<std::string_view, PlyWriter::Callback,
                                        std::optional<PlyWriter::SizeType>>>>>
      callbacks;
  for (const auto& element : properties) {
    std::vector<std::tuple<std::string_view, PlyWriter::Callback,
                           std::optional<PlyWriter::SizeType>>>
        entries;
    for (const auto& property : element.second.second) {
      if (std::visit([](auto ptr) { return !ptr; }, property.second)) {
        continue;
      }

      std::optional<PlyWriter::SizeType> size_type;
      if (property.second.index() & 0x1u) {
        auto result =
            ply_writer.GetPropertyListSizeType(element.first, property.first);
        if (!result) {
          return std::unexpected(result.error());
        }
        size_type = *result;
      }

      entries.emplace_back(property.first, property.second, size_type);
    }
    callbacks.emplace_back(element.first, element.second.first,
                           std::move(entries));
  }

  return callbacks;
}

std::expected<std::vector<std::tuple<
                  std::string_view, uint64_t,
                  std::vector<std::tuple<std::string_view, PlyWriter::Callback,
                                         std::optional<PlyWriter::SizeType>>>>>,
              std::string_view>
WriteHeader(
    const PlyWriter& ply_writer, std::ostream& stream,
    const std::map<
        std::string_view,
        std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info, std::string_view format) {
  static const std::array<std::string_view, 16> type_names = {
      "char", "char", "uchar", "uchar", "short", "short", "ushort", "ushort",
      "int",  "int",  "uint",  "uint",  "float", "float", "double", "double"};

  auto properties_valid = ValidateProperties(properties);
  if (!properties_valid) {
    return std::unexpected(properties_valid.error());
  }

  auto comments_valid = ValidateComments(comments);
  if (!comments_valid) {
    return std::unexpected(comments_valid.error());
  }

  auto object_info_valid = ValidateComments(object_info);
  if (!object_info_valid) {
    return std::unexpected(
        "A obj_info may not contain line feed or carriage return");
  }

  auto callbacks = GetListSizes(ply_writer, properties);
  if (!callbacks) {
    return std::unexpected(callbacks.error());
  }

  stream << "ply\rformat " << format << " 1.0\r";
  if (!stream) {
    return std::unexpected(WriteFailure());
  }

  for (const auto& comment : comments) {
    stream << "comment " << comment << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }
  }

  for (const auto& info : object_info) {
    stream << "obj_info " << info << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }
  }

  for (const auto& element : *callbacks) {
    stream << "element " << std::get<0>(element) << " " << std::get<1>(element)
           << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }

    for (const auto& property : std::get<2>(element)) {
      if (std::get<2>(property).has_value()) {
        stream << "property list " << type_names.at(*std::get<2>(property))
               << " " << type_names.at(std::get<1>(property).index()) << " "
               << std::get<0>(property) << "\r";
      } else {
        stream << "property " << type_names.at(std::get<1>(property).index())
               << " " << std::get<0>(property) << "\r";
      }

      if (!stream) {
        return std::unexpected(WriteFailure());
      }
    }
  }

  stream << "end_header\r";
  if (!stream) {
    return std::unexpected(WriteFailure());
  }

  return callbacks;
}

template <std::floating_point T>
std::string_view SerializeFP(std::stringstream& stream, T value) {
  stream.str("");

  int log = static_cast<int>(std::log10(std::abs(value))) + 1;
  int num_digits = std::max(std::numeric_limits<T>::max_digits10 - log, 0);
  stream << std::fixed << std::setprecision(num_digits) << value;

  std::string_view result = stream.view();

  size_t dot = result.find(".");
  if (dot == std::string_view::npos) {
    return result;
  }

  result = result.substr(0u, result.find_last_not_of("0") + 1u);
  if (result.back() == '.') {
    result.remove_suffix(1u);
  }

  return result;
}

std::expected<void, std::string_view> ValidateListSize(PlyWriter::SizeType type,
                                                       size_t size) {
  static const std::array<uint32_t, 16> kSizes = {
      std::numeric_limits<int8_t>::max(),
      0u,
      std::numeric_limits<uint8_t>::max(),
      0u,
      std::numeric_limits<int16_t>::max(),
      0u,
      std::numeric_limits<uint16_t>::max(),
      0u,
      std::numeric_limits<int32_t>::max(),
      0u,
      std::numeric_limits<uint32_t>::max(),
      0u,
      0u,
      0u,
      0u,
      0u};

  if (kSizes.at(type) < size) {
    return std::unexpected(
        "The list was too big to be represented with the selected index type");
  }

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness>
std::expected<void, std::string_view> WriteToBinaryImpl(PlyWriter& ply_writer,
                                                        std::ostream& stream) {
  std::string_view format;
  if constexpr (Endianness == std::endian::big) {
    format = "binary_big_endian";
  } else {
    format = "binary_little_endian";
  }
  std::map<std::string_view,
           std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>
      property_callbacks;
  std::span<const std::string> comments;
  std::span<const std::string> object_info;
  auto result = ply_writer.Start(property_callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  auto callbacks = WriteHeader(ply_writer, stream, property_callbacks, comments,
                               object_info, format);
  if (!callbacks) {
    return std::unexpected(callbacks.error());
  }

  for (const auto& element : *callbacks) {
    for (uint64_t e = 0u; e < std::get<1>(element); e++) {
      for (const auto& property : std::get<2>(element)) {
        auto result = std::visit(
            [&](const auto& callback) -> std::expected<void, std::string_view> {
              auto result = (ply_writer.*callback)(std::get<0>(element),
                                                   std::get<0>(property));
              if (!result) {
                return std::unexpected(result.error());
              }

              if constexpr (std::is_class<
                                std::decay_t<decltype(*result)>>::value) {
                auto size_valid =
                    ValidateListSize(*std::get<2>(property), result->size());
                if (!size_valid) {
                  return size_valid;
                }

                uint8_t size8;
                uint16_t size16;
                uint32_t size32;
                switch (std::get<2>(property).value()) {
                  case PlyWriter::UINT8:
                    size8 = static_cast<uint8_t>(result->size());
                    stream.write(reinterpret_cast<char*>(&size8),
                                 sizeof(size8));
                    break;
                  case PlyWriter::UINT16:
                    size16 = static_cast<uint16_t>(result->size());

                    if (Endianness != std::endian::native) {
                      size16 = std::byteswap(size16);
                    }

                    stream.write(reinterpret_cast<char*>(&size16),
                                 sizeof(size16));
                    break;
                  case PlyWriter::UINT32:
                    size32 = static_cast<uint32_t>(result->size());

                    if (Endianness != std::endian::native) {
                      size32 = std::byteswap(size32);
                    }

                    stream.write(reinterpret_cast<char*>(&size32),
                                 sizeof(size32));
                    break;
                }

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }

                for (auto entry : *result) {
                  if constexpr (std::is_same<float, decltype(entry)>::value) {
                    auto to_write = std::bit_cast<uint32_t>(entry);

                    if (Endianness != std::endian::native) {
                      to_write = std::byteswap(to_write);
                    }

                    stream.write(reinterpret_cast<char*>(&to_write),
                                 sizeof(to_write));
                  } else if constexpr (std::is_same<double,
                                                    decltype(entry)>::value) {
                    auto to_write = std::bit_cast<uint64_t>(entry);

                    if (Endianness != std::endian::native) {
                      to_write = std::byteswap(to_write);
                    }

                    stream.write(reinterpret_cast<char*>(&to_write),
                                 sizeof(to_write));
                  } else {
                    if (Endianness != std::endian::native) {
                      entry = std::byteswap(entry);
                    }

                    stream.write(reinterpret_cast<char*>(&entry),
                                 sizeof(entry));
                  }

                  if (!stream) {
                    return std::unexpected(WriteFailure());
                  }
                }
              } else if constexpr (std::is_same<
                                       float, std::decay_t<decltype(*result)>>::
                                       value) {
                auto to_write = std::bit_cast<uint32_t>(*result);

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }
              } else if constexpr (std::is_same<
                                       double, std::decay_t<
                                                   decltype(*result)>>::value) {
                auto to_write = std::bit_cast<uint64_t>(*result);

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }
              } else {
                auto to_write = *result;

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }
              }

              return std::expected<void, std::string_view>();
            },
            std::get<1>(property));

        if (!result) {
          return result;
        }
      }
    }
  }

  return std::expected<void, std::string_view>();
}

}  // namespace

std::expected<void, std::string_view> PlyWriter::WriteTo(std::ostream& stream) {
  if constexpr (std::endian::native == std::endian::big) {
    return WriteToBigEndian(stream);
  } else {
    return WriteToLittleEndian(stream);
  }
}

std::expected<void, std::string_view> PlyWriter::WriteToASCII(
    std::ostream& stream) {
  std::map<std::string_view,
           std::pair<uint64_t, std::map<std::string_view, Callback>>>
      property_callbacks;
  std::span<const std::string> comments;
  std::span<const std::string> object_info;
  auto result = Start(property_callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  auto callbacks = WriteHeader(*this, stream, property_callbacks, comments,
                               object_info, "ascii");
  if (!callbacks) {
    return std::unexpected(callbacks.error());
  }

  std::stringstream fp_stream_storage;
  for (const auto& element : *callbacks) {
    for (size_t i = 0; i < std::get<1>(element); i++) {
      bool first = true;
      for (const auto& property : std::get<2>(element)) {
        if (!first) {
          stream << " ";
          if (!stream) {
            return std::unexpected(WriteFailure());
          }
        }

        first = false;

        auto result = std::visit(
            [&](const auto& callback) -> std::expected<void, std::string_view> {
              auto result = (this->*callback)(std::get<0>(element),
                                              std::get<0>(property));
              if (!result) {
                return std::unexpected(result.error());
              }

              if constexpr (std::is_class<
                                std::decay_t<decltype(*result)>>::value) {
                auto size_valid =
                    ValidateListSize(*std::get<2>(property), result->size());
                if (!size_valid) {
                  return size_valid;
                }

                stream << result->size();
                if (!stream) {
                  return std::unexpected(WriteFailure());
                }

                for (auto entry : *result) {
                  if constexpr (std::is_floating_point<
                                    decltype(entry)>::value) {
                    if (!std::isfinite(entry)) {
                      return std::unexpected(FloatingPointError());
                    }

                    stream << " " << SerializeFP(fp_stream_storage, entry);
                  } else {
                    stream << " " << +entry;
                  }

                  if (!stream) {
                    return std::unexpected(WriteFailure());
                  }
                }
              } else if constexpr (std::is_floating_point<std::decay_t<
                                       decltype(*result)>>::value) {
                if (!std::isfinite(*result)) {
                  return std::unexpected(FloatingPointError());
                }

                stream << SerializeFP(fp_stream_storage, *result);

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }
              } else {
                stream << +*result;

                if (!stream) {
                  return std::unexpected(WriteFailure());
                }
              }

              return std::expected<void, std::string_view>();
            },
            std::get<1>(property));

        if (!result) {
          return result;
        }
      }

      stream << "\r";
      if (!stream) {
        return std::unexpected(WriteFailure());
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> PlyWriter::WriteToBigEndian(
    std::ostream& stream) {
  return WriteToBinaryImpl<std::endian::big>(*this, stream);
}

std::expected<void, std::string_view> PlyWriter::WriteToLittleEndian(
    std::ostream& stream) {
  return WriteToBinaryImpl<std::endian::little>(*this, stream);
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

// Static assertions to ensure variants of Callback are properly ordered
static_assert(PlyWriter::Callback(PlyWriter::Int8PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT8));
static_assert(PlyWriter::Callback(PlyWriter::Int8PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT8_LIST));
static_assert(PlyWriter::Callback(PlyWriter::UInt8PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::UINT8));
static_assert(PlyWriter::Callback(PlyWriter::UInt8PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::UINT8_LIST));
static_assert(PlyWriter::Callback(PlyWriter::Int16PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT16));
static_assert(PlyWriter::Callback(PlyWriter::Int16PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT16_LIST));
static_assert(PlyWriter::Callback(PlyWriter::UInt16PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::UINT16));
static_assert(
    PlyWriter::Callback(PlyWriter::UInt16PropertyListCallback(nullptr))
        .index() == static_cast<size_t>(PropertyType::UINT16_LIST));
static_assert(PlyWriter::Callback(PlyWriter::Int32PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT32));
static_assert(PlyWriter::Callback(PlyWriter::Int32PropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::INT32_LIST));
static_assert(PlyWriter::Callback(PlyWriter::UInt32PropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::UINT32));
static_assert(
    PlyWriter::Callback(PlyWriter::UInt32PropertyListCallback(nullptr))
        .index() == static_cast<size_t>(PropertyType::UINT32_LIST));
static_assert(PlyWriter::Callback(PlyWriter::FloatPropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::FLOAT));
static_assert(PlyWriter::Callback(PlyWriter::FloatPropertyListCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::FLOAT_LIST));
static_assert(PlyWriter::Callback(PlyWriter::DoublePropertyCallback(nullptr))
                  .index() == static_cast<size_t>(PropertyType::DOUBLE));
static_assert(
    PlyWriter::Callback(PlyWriter::DoublePropertyListCallback(nullptr))
        .index() == static_cast<size_t>(PropertyType::DOUBLE_LIST));

}  // namespace plyodine
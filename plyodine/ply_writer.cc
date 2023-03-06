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

static const std::array<std::string_view, 16> type_strings = {
    "char", "char", "uchar", "uchar", "short", "short", "ushort", "ushort",
    "int",  "int",  "uint",  "uint",  "float", "float", "double", "double"};

enum ListSize {
  LIST_SIZE_UINT8 = 2u,
  LIST_SIZE_UINT16 = 6u,
  LIST_SIZE_UINT32 = 10u,
};

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
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties) {
  for (const auto& [element_name, element_properties] : properties) {
    auto element_name_valid = ValidateName(element_name);
    if (!element_name_valid) {
      return element_name_valid;
    }

    std::optional<size_t> property_size;
    for (const auto& [property_name, property] : element_properties) {
      auto property_name_valid = ValidateName(property_name);
      if (!property_name_valid) {
        return property_name_valid;
      }

      if (!property_size) {
        property_size = property.size();
      } else if (property_size != property.size()) {
        return std::unexpected(
            "All properties of an element must have the same size");
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ValidateComments(
    std::span<const std::string_view> comments) {
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

std::expected<std::map<std::string_view, std::map<std::string_view, ListSize>>,
              std::string_view>
GetListSizes(const std::map<std::string_view,
                            std::map<std::string_view, Property>>& properties) {
  std::map<std::string_view, std::map<std::string_view, ListSize>> list_sizes;
  for (const auto& element : properties) {
    for (const auto& property : element.second) {
      bool success = std::visit(
          [&](const auto& entries) {
            if constexpr (std::is_class<
                              std::decay_t<decltype(entries[0])>>::value) {
              size_t max_size = 0;
              for (const auto& entry : entries) {
                max_size = std::max(max_size, entry.size());
              }

              if (max_size <= std::numeric_limits<uint8_t>::max()) {
                list_sizes[element.first][property.first] = LIST_SIZE_UINT8;
              } else if (max_size <= std::numeric_limits<uint16_t>::max()) {
                list_sizes[element.first][property.first] = LIST_SIZE_UINT16;
              } else if (max_size <= std::numeric_limits<uint32_t>::max()) {
                list_sizes[element.first][property.first] = LIST_SIZE_UINT32;
              } else {
                return false;
              }
            }

            return true;
          },
          property.second);

      if (!success) {
        return std::unexpected("A property list contained too many values");
      }
    }
  }

  return std::expected<
      std::map<std::string_view, std::map<std::string_view, ListSize>>,
      std::string_view>(std::move(list_sizes));
}

std::expected<std::map<std::string_view, std::map<std::string_view, ListSize>>,
              std::string_view>
WriteHeader(std::ostream& stream,
            const std::map<std::string_view,
                           std::map<std::string_view, Property>>& properties,
            std::span<const std::string_view> comments,
            std::string_view format) {
  auto properties_valid = ValidateProperties(properties);
  if (!properties_valid) {
    return std::unexpected(properties_valid.error());
  }

  auto comments_valid = ValidateComments(comments);
  if (!comments_valid) {
    return std::unexpected(comments_valid.error());
  }

  auto list_sizes = GetListSizes(properties);
  if (!list_sizes) {
    return std::unexpected(list_sizes.error());
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

  for (const auto& element : properties) {
    bool first = true;
    for (const auto& property : element.second) {
      if (first) {
        stream << "element " << element.first << " " << property.second.size()
               << "\r";
        if (!stream) {
          return std::unexpected(WriteFailure());
        }
        first = false;
      }

      std::visit(
          [&](const auto& entries) {
            if constexpr (std::is_class<
                              std::decay_t<decltype(entries[0])>>::value) {
              stream << "property list "
                     << type_strings.at(
                            list_sizes->at(element.first).at(property.first))
                     << " " << type_strings.at(property.second.type()) << " "
                     << property.first << "\r";
            } else {
              stream << "property " << type_strings.at(property.second.type())
                     << " " << property.first << "\r";
            }
          },
          property.second);

      if (!stream) {
        return std::unexpected(WriteFailure());
      }
    }
  }

  stream << "end_header\r";
  if (!stream) {
    return std::unexpected(WriteFailure());
  }

  return std::move(*list_sizes);
}

template <std::endian Endianness>
std::expected<void, std::string_view> WriteToBinaryImpl(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments) {
  std::string_view format;
  if constexpr (Endianness == std::endian::big) {
    format = "binary_big_endian";
  } else {
    format = "binary_little_endian";
  }

  auto list_sizes = WriteHeader(stream, properties, comments, format);
  if (!list_sizes) {
    return std::unexpected(list_sizes.error());
  }

  for (const auto& element : properties) {
    if (element.second.empty()) {
      continue;
    }

    std::vector<ListSize> element_list_sizes;
    auto list_sizes_iter = list_sizes->find(element.first);
    if (list_sizes_iter != list_sizes->end()) {
      for (const auto& entry : list_sizes_iter->second) {
        element_list_sizes.push_back(entry.second);
      }
    }

    size_t num_elements = element.second.begin()->second.size();
    for (size_t i = 0; i < num_elements; i++) {
      size_t list_size_index = 0;
      for (const auto& property : element.second) {
        std::visit(
            [&](const auto& entries) {
              if constexpr (std::is_class<
                                std::decay_t<decltype(entries[0])>>::value) {
                auto list_size = element_list_sizes.at(list_size_index++);

                uint8_t size8;
                uint16_t size16;
                uint32_t size32;
                switch (list_size) {
                  case LIST_SIZE_UINT8:
                    size8 = static_cast<uint8_t>(entries[i].size());
                    stream.write(reinterpret_cast<char*>(&size8),
                                 sizeof(size8));
                    break;
                  case LIST_SIZE_UINT16:
                    size16 = static_cast<uint16_t>(entries[i].size());

                    if (Endianness != std::endian::native) {
                      size16 = std::byteswap(size16);
                    }

                    stream.write(reinterpret_cast<char*>(&size16),
                                 sizeof(size16));
                    break;
                  case LIST_SIZE_UINT32:
                    size32 = static_cast<uint32_t>(entries[i].size());

                    if (Endianness != std::endian::native) {
                      size32 = std::byteswap(size32);
                    }

                    stream.write(reinterpret_cast<char*>(&size32),
                                 sizeof(size32));
                    break;
                }

                if (!stream) {
                  return;
                }

                for (auto entry : entries[i]) {
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
                }
              } else if constexpr (std::is_same<
                                       float,
                                       std::decay_t<decltype(entries[0])>>::
                                       value) {
                auto to_write = std::bit_cast<uint32_t>(entries[i]);

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));
              } else if constexpr (std::is_same<
                                       double,
                                       std::decay_t<decltype(entries[0])>>::
                                       value) {
                auto to_write = std::bit_cast<uint64_t>(entries[i]);

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));
              } else {
                auto to_write = entries[i];

                if (Endianness != std::endian::native) {
                  to_write = std::byteswap(to_write);
                }

                stream.write(reinterpret_cast<char*>(&to_write),
                             sizeof(to_write));
              }
            },
            property.second);

        if (!stream) {
          return std::unexpected(WriteFailure());
        }
      }
    }
  }

  return std::expected<void, std::string_view>();
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

}  // namespace

std::expected<void, std::string_view> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments) {
  auto list_sizes = WriteHeader(stream, properties, comments, "ascii");
  if (!list_sizes) {
    return std::unexpected(list_sizes.error());
  }

  std::stringstream fp_stream_storage;
  for (const auto& element : properties) {
    if (element.second.empty()) {
      continue;
    }

    size_t num_elements = element.second.begin()->second.size();
    for (size_t i = 0; i < num_elements; i++) {
      bool first = true;
      for (const auto& property : element.second) {
        if (!first) {
          stream << " ";
          if (!stream) {
            return std::unexpected(WriteFailure());
          }
        }

        first = false;

        bool float_error = std::visit(
            [&](const auto& entries) {
              if constexpr (std::is_class<
                                std::decay_t<decltype(entries[0])>>::value) {
                stream << entries[i].size();
                if (!stream) {
                  return false;
                }

                for (auto entry : entries[i]) {
                  if constexpr (std::is_floating_point<
                                    decltype(entry)>::value) {
                    if (!std::isfinite(entry)) {
                      return true;
                    }

                    stream << " " << SerializeFP(fp_stream_storage, entry);
                  } else {
                    stream << " " << +entry;
                  }
                }
              } else if constexpr (std::is_floating_point<std::decay_t<
                                       decltype(entries[0])>>::value) {
                if (!std::isfinite(entries[i])) {
                  return true;
                }

                stream << SerializeFP(fp_stream_storage, entries[i]);
              } else {
                stream << +entries[i];
              }

              return false;
            },
            property.second);

        if (float_error) {
          return std::unexpected(
              "Only finite floating point values may be serialized to an ASCII "
              "output");
        }

        if (!stream) {
          return std::unexpected(WriteFailure());
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

std::expected<void, std::string_view> WriteToBinary(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments) {
  return WriteToBinaryImpl<std::endian::native>(stream, properties, comments);
}

std::expected<void, std::string_view> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments) {
  return WriteToBinaryImpl<std::endian::big>(stream, properties, comments);
}

std::expected<void, std::string_view> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string_view> comments) {
  return WriteToBinaryImpl<std::endian::little>(stream, properties, comments);
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
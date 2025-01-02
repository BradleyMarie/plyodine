#include "plyodine/ply_writer.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <functional>
#include <generator>
#include <iomanip>
#include <ios>
#include <limits>
#include <map>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

namespace {

enum class ErrorCode : int {
  MIN_VALUE = 1,
  BAD_STREAM = 1,
  COMMENT_CONTAINS_NEWLINE = 2,
  OBJ_INFO_CONTAINS_NEWLINE = 3,
  EMPTY_NAME_SPECIFIED = 4,
  NAME_CONTAINED_INVALID_CHARACTERS = 5,
  LIST_INDEX_TOO_SMALL = 6,
  ASCII_FLOAT_NOT_FINITE = 7,
  NOT_ENOUGH_VALUES = 8,
  MAX_VALUE = 8,
};

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
  std::error_condition default_error_condition(int value) const noexcept;
} kErrorCategory;

const char* ErrorCategory::name() const noexcept {
  return "plyodine::PlyWriter";
}

std::string ErrorCategory::message(int condition) const {
  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "Output stream must be in good state";
    case ErrorCode::COMMENT_CONTAINS_NEWLINE:
      return "A comment may not contain line feed or carriage return";
    case ErrorCode::OBJ_INFO_CONTAINS_NEWLINE:
      return "An obj_info may not contain line feed or carriage return";
    case ErrorCode::EMPTY_NAME_SPECIFIED:
      return "Names of properties and elements may not be empty";
    case ErrorCode::NAME_CONTAINED_INVALID_CHARACTERS:
      return "Names of properties and elements may only contain graphic "
             "characters";
    case ErrorCode::LIST_INDEX_TOO_SMALL:
      return "The list was too big to be represented with the selected size "
             "type";
    case ErrorCode::ASCII_FLOAT_NOT_FINITE:
      return "Only finite floating point values may be serialized to an ASCII "
             "output";
    case ErrorCode::NOT_ENOUGH_VALUES:
      return "A property generator did not produce enough values to cover all "
             "every associated element instance in the output";
  };

  return "Unknown Error";
}

std::error_condition ErrorCategory::default_error_condition(
    int value) const noexcept {
  if (value < static_cast<int>(ErrorCode::MIN_VALUE) ||
      value > static_cast<int>(ErrorCode::MAX_VALUE)) {
    return std::error_condition(value, *this);
  }

  return std::make_error_condition(std::errc::invalid_argument);
}

std::error_code make_error_code(ErrorCode code) {
  return std::error_code(static_cast<int>(code), kErrorCategory);
}

}  // namespace

namespace std {

template <>
struct is_error_code_enum<ErrorCode> : true_type {};

}  // namespace std

namespace plyodine {
namespace {

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::stringstream>
    Context;

std::error_code ValidateName(const std::string& name) {
  if (name.empty()) {
    return ErrorCode::EMPTY_NAME_SPECIFIED;
  }

  for (char c : name) {
    if (!std::isgraph(c)) {
      return ErrorCode::NAME_CONTAINED_INVALID_CHARACTERS;
    }
  }

  return std::error_code();
}

bool ValidateComment(const std::string& comment) {
  for (char c : comment) {
    if (c == '\r' || c == '\n') {
      return false;
    }
  }

  return true;
}

template <std::integral T>
std::error_code SerializeASCII(std::ostream& output, std::stringstream& storage,
                               uint32_t list_capacity, T value) {
  output << +value;
  if (!output) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::floating_point T>
std::error_code SerializeASCII(std::ostream& output, std::stringstream& storage,
                               uint32_t list_capacity, T value) {
  if (!std::isfinite(value)) {
    return ErrorCode::ASCII_FLOAT_NOT_FINITE;
  }

  storage.str("");

  int log = static_cast<int>(std::log10(std::abs(value))) + 1;
  int num_digits = std::max(std::numeric_limits<T>::max_digits10 - log, 0);
  storage << std::fixed << std::setprecision(num_digits) << value;

  std::string_view result = storage.view();

  size_t dot = result.find(".");
  if (dot != std::string_view::npos) {
    result = result.substr(0u, result.find_last_not_of("0") + 1u);
    if (result.back() == '.') {
      result.remove_suffix(1u);
    }
  }

  if (!output.write(result.data(), result.size())) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <typename T>
std::error_code SerializeASCII(std::ostream& output, std::stringstream& storage,
                               uint32_t list_capacity,
                               std::span<const T> values) {
  if (list_capacity < values.size()) {
    return ErrorCode::LIST_INDEX_TOO_SMALL;
  }

  if (list_capacity == std::numeric_limits<uint8_t>::max()) {
    if (std::error_code error =
            SerializeASCII(output, storage, list_capacity,
                           static_cast<uint8_t>(values.size()));
        error) {
      return error;
    }
  } else if (list_capacity == std::numeric_limits<uint16_t>::max()) {
    if (std::error_code error =
            SerializeASCII(output, storage, list_capacity,
                           static_cast<uint16_t>(values.size()));
        error) {
      return error;
    }
  } else if (std::error_code error =
                 SerializeASCII(output, storage, list_capacity,
                                static_cast<uint32_t>(values.size()));
             error) {
    return error;
  }

  for (const auto& entry : values) {
    if (!output.put(' ')) {
      return std::io_errc::stream;
    }

    if (std::error_code error =
            SerializeASCII(output, storage, list_capacity, entry);
        error) {
      return error;
    }
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code SerializeBinary(std::ostream& output, uint32_t list_capacity,
                                T value) {
  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  output.write(reinterpret_cast<char*>(&value), sizeof(value));
  if (!output) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::endian Endianness, std::floating_point T>
std::error_code SerializeBinary(std::ostream& output, uint32_t list_capacity,
                                T value) {
  auto entry = std::bit_cast<
      std::conditional_t<std::is_same<T, float>::value, uint32_t, uintmax_t>>(
      value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  output.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!output) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::endian Endianness, typename T>
std::error_code SerializeBinary(std::ostream& output, uint32_t list_capacity,
                                std::span<const T> values) {
  if (list_capacity < values.size()) {
    return ErrorCode::LIST_INDEX_TOO_SMALL;
  }

  if (list_capacity == std::numeric_limits<uint8_t>::max()) {
    if (std::error_code error = SerializeBinary<Endianness>(
            output, list_capacity, static_cast<uint8_t>(values.size()));
        error) {
      return error;
    }
  } else if (list_capacity == std::numeric_limits<uint16_t>::max()) {
    if (std::error_code error = SerializeBinary<Endianness>(
            output, list_capacity, static_cast<uint16_t>(values.size()));
        error) {
      return error;
    }
  } else if (std::error_code error = SerializeBinary<Endianness>(
                 output, list_capacity, static_cast<uint32_t>(values.size()));
             error) {
    return error;
  }

  for (const auto& entry : values) {
    if (std::error_code error =
            SerializeBinary<Endianness>(output, list_capacity, entry);
        error) {
      return error;
    }
  }

  return std::error_code();
}

template <typename T>
std::error_code WriteHeader(
    std::ostream& output, std::string_view format,
    std::map<std::string, uintmax_t>& num_element_instances,
    const std::map<std::string, std::map<std::string, T>>&
        generators_and_list_details,
    const std::vector<std::string>& comments,
    const std::vector<std::string>& object_info) {
  static constexpr std::string_view header_prefix = "ply\rformat ";
  static constexpr std::string_view version_suffix = " 1.0\r";
  static constexpr std::string_view comment_prefix = "comment ";
  static constexpr std::string_view obj_info_prefix = "obj_info ";
  static constexpr std::string_view element_prefix = "element ";
  static constexpr std::string_view property_prefix = "property ";
  static constexpr std::string_view data_type_names[8] = {
      "char ", "uchar ", "short ", "ushort ",
      "int ",  "uint ",  "float ", "double "};
  static constexpr std::string_view header_suffix = "end_header\r";

  if (!output.write(header_prefix.data(), header_prefix.size()) ||
      !output.write(format.data(), format.size()) ||
      !output.write(version_suffix.data(), version_suffix.size())) {
    return std::io_errc::stream;
  }

  for (const auto& comment : comments) {
    if (!ValidateComment(comment)) {
      return ErrorCode::COMMENT_CONTAINS_NEWLINE;
    }

    if (!output.write(comment_prefix.data(), comment_prefix.size()) ||
        !output.write(comment.data(), comment.size()) || !output.put('\r')) {
      return std::io_errc::stream;
    }
  }

  for (const auto& info : object_info) {
    if (!ValidateComment(info)) {
      return ErrorCode::OBJ_INFO_CONTAINS_NEWLINE;
    }

    if (!output.write(obj_info_prefix.data(), obj_info_prefix.size()) ||
        !output.write(info.data(), info.size()) || !output.put('\r')) {
      return std::io_errc::stream;
    }
  }

  for (const auto& [element_name, element_generators] :
       generators_and_list_details) {
    if (std::error_code error = ValidateName(element_name); error) {
      return error;
    }

    uintmax_t num_instances = num_element_instances[element_name];

    if (!output.write(element_prefix.data(), element_prefix.size()) ||
        !output.write(element_name.data(), element_name.size()) ||
        !output.put(' ') || !(output << num_instances) || !output.put('\r')) {
      return std::io_errc::stream;
    }

    for (const auto& [property_name, generator_and_details] :
         element_generators) {
      if (std::error_code error = ValidateName(property_name); error) {
        return error;
      }

      if (!output.write(property_prefix.data(), property_prefix.size())) {
        return std::io_errc::stream;
      }

      if (generator_and_details.second != nullptr &&
          !output.write(generator_and_details.second->first.data(),
                        generator_and_details.second->first.size())) {
        return std::io_errc::stream;
      }

      const std::string_view& data_type_name =
          data_type_names[generator_and_details.first.index() >> 1u];
      if (!output.write(data_type_name.data(), data_type_name.size()) ||
          !output.write(property_name.data(), property_name.size()) ||
          !output.put('\r')) {
        return std::io_errc::stream;
      }
    }
  }

  if (!output.write(header_suffix.data(), header_suffix.size())) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

typedef std::pair<std::string_view, uint32_t> ListTypeNameAndCapacity;
static constexpr ListTypeNameAndCapacity list_type_name_and_capacity[3] = {
    {"list uchar ", std::numeric_limits<uint8_t>::max()},
    {"list ushort ", std::numeric_limits<uint16_t>::max()},
    {"list uint ", std::numeric_limits<uint32_t>::max()},
};

}  // namespace

std::error_code PlyWriter::WriteTo(std::ostream& stream) const {
  if constexpr (std::endian::native == std::endian::big) {
    return WriteToBigEndian(stream);
  } else {
    return WriteToLittleEndian(stream);
  }
}

std::error_code PlyWriter::WriteToASCII(std::ostream& stream) const {
  if (!stream) {
    return ErrorCode::BAD_STREAM;
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, ValueGenerator>> generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, generators, comments, object_info);
      error) {
    return error;
  }

  std::map<std::string,
           std::map<std::string,
                    std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>>
      generators_with_capacities;
  for (auto& [element_name, elements] : generators) {
    std::map<std::string,
             std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>&
        element_generators = generators_with_capacities[element_name];
    for (auto& [property_name, generator] : elements) {
      const ListTypeNameAndCapacity* name_and_capacity = nullptr;
      if (generator.index() & 1u) {
        ListSizeType list_size_type =
            std::min(GetPropertyListSizeType(element_name, property_name),
                     ListSizeType::UINT32);
        name_and_capacity =
            &list_type_name_and_capacity[static_cast<size_t>(list_size_type)];
      }
      element_generators.try_emplace(property_name, std::move(generator),
                                     name_and_capacity);
    }
  }

  if (std::error_code error =
          WriteHeader(stream, "ascii", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  std::stringstream storage;
  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      uint32_t list_capacity = (generator_and_list_details.second != nullptr)
                                   ? generator_and_list_details.second->second
                                   : 0u;

      std::move_only_function<std::error_code()> add_property_func = std::visit(
          [&](auto& generator) -> std::move_only_function<std::error_code()> {
            return [iter = generator.begin(), end = generator.end(), &stream,
                    list_capacity, &storage]() mutable -> std::error_code {
              if (iter == end) {
                return std::error_code(ErrorCode::NOT_ENOUGH_VALUES);
              }

              if (std::error_code error =
                      SerializeASCII(stream, storage, list_capacity, *iter);
                  error) {
                return error;
              }

              iter++;

              return std::error_code();
            };
          },
          generator_and_list_details.first);

      add_next_property.push_back(std::move(add_property_func));
    }

    for (uintmax_t i = 0; i < num_element_instances[element_name]; i++) {
      bool first = true;
      for (auto& add_next_property_func : add_next_property) {
        if (!first && !stream.put(' ')) {
          return std::io_errc::stream;
        }

        if (std::error_code error = add_next_property_func(); error) {
          return error;
        }

        first = false;
      }

      if (!stream.put('\r')) {
        return std::io_errc::stream;
      }
    }
  }

  return std::error_code();
}

std::error_code PlyWriter::WriteToBigEndian(std::ostream& stream) const {
  if (!stream) {
    return ErrorCode::BAD_STREAM;
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, ValueGenerator>> generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, generators, comments, object_info);
      error) {
    return error;
  }

  std::map<std::string,
           std::map<std::string,
                    std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>>
      generators_with_capacities;
  for (auto& [element_name, elements] : generators) {
    std::map<std::string,
             std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>&
        element_generators = generators_with_capacities[element_name];
    for (auto& [property_name, generator] : elements) {
      const ListTypeNameAndCapacity* name_and_capacity = nullptr;
      if (generator.index() & 1u) {
        ListSizeType list_size_type =
            std::min(GetPropertyListSizeType(element_name, property_name),
                     ListSizeType::UINT32);
        name_and_capacity =
            &list_type_name_and_capacity[static_cast<size_t>(list_size_type)];
      }
      element_generators.try_emplace(property_name, std::move(generator),
                                     name_and_capacity);
    }
  }

  if (std::error_code error =
          WriteHeader(stream, "binary_big_endian", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      uint32_t list_capacity = (generator_and_list_details.second != nullptr)
                                   ? generator_and_list_details.second->second
                                   : 0u;

      std::move_only_function<std::error_code()> add_property_func = std::visit(
          [&](auto& generator) -> std::move_only_function<std::error_code()> {
            return [iter = generator.begin(), end = generator.end(), &stream,
                    list_capacity]() mutable -> std::error_code {
              if (iter == end) {
                return std::error_code(ErrorCode::NOT_ENOUGH_VALUES);
              }

              if (std::error_code error = SerializeBinary<std::endian::big>(
                      stream, list_capacity, *iter);
                  error) {
                return error;
              }

              iter++;

              return std::error_code();
            };
          },
          generator_and_list_details.first);

      add_next_property.push_back(std::move(add_property_func));
    }

    for (uintmax_t i = 0; i < num_element_instances[element_name]; i++) {
      for (auto& add_next_property_func : add_next_property) {
        if (std::error_code error = add_next_property_func(); error) {
          return error;
        }
      }
    }
  }

  return std::error_code();
}

std::error_code PlyWriter::WriteToLittleEndian(std::ostream& stream) const {
  if (!stream) {
    return ErrorCode::BAD_STREAM;
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, ValueGenerator>> generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error =
          Start(num_element_instances, generators, comments, object_info);
      error) {
    return error;
  }

  std::map<std::string,
           std::map<std::string,
                    std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>>
      generators_with_capacities;
  for (auto& [element_name, elements] : generators) {
    std::map<std::string,
             std::pair<ValueGenerator, const ListTypeNameAndCapacity*>>&
        element_generators = generators_with_capacities[element_name];
    for (auto& [property_name, generator] : elements) {
      const ListTypeNameAndCapacity* name_and_capacity = nullptr;
      if (generator.index() & 1u) {
        ListSizeType list_size_type =
            std::min(GetPropertyListSizeType(element_name, property_name),
                     ListSizeType::UINT32);
        name_and_capacity =
            &list_type_name_and_capacity[static_cast<size_t>(list_size_type)];
      }
      element_generators.try_emplace(property_name, std::move(generator),
                                     name_and_capacity);
    }
  }

  if (std::error_code error =
          WriteHeader(stream, "binary_little_endian", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      uint32_t list_capacity = (generator_and_list_details.second != nullptr)
                                   ? generator_and_list_details.second->second
                                   : 0u;

      std::move_only_function<std::error_code()> add_property_func = std::visit(
          [&](auto& generator) -> std::move_only_function<std::error_code()> {
            return [iter = generator.begin(), end = generator.end(), &stream,
                    list_capacity]() mutable -> std::error_code {
              if (iter == end) {
                return std::error_code(ErrorCode::NOT_ENOUGH_VALUES);
              }

              if (std::error_code error = SerializeBinary<std::endian::little>(
                      stream, list_capacity, *iter);
                  error) {
                return error;
              }

              iter++;

              return std::error_code();
            };
          },
          generator_and_list_details.first);

      add_next_property.push_back(std::move(add_property_func));
    }

    for (uintmax_t i = 0; i < num_element_instances[element_name]; i++) {
      for (auto& add_next_property_func : add_next_property) {
        if (std::error_code error = add_next_property_func(); error) {
          return error;
        }
      }
    }
  }

  return std::error_code();
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
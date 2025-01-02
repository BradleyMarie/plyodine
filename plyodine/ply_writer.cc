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
  ELEMENT_HAS_NO_PROPERTIES = 4,
  PROPERTY_HAS_NO_INSTANCES = 5,
  EMPTY_NAME_SPECIFIED = 6,
  NAME_CONTAINED_INVALID_CHARACTERS = 7,
  LIST_INDEX_TOO_SMALL = 8,
  ASCII_FLOAT_NOT_FINITE = 9,
  NOT_ENOUGH_VALUES = 10,
  MAX_VALUE = 10,
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
    case ErrorCode::ELEMENT_HAS_NO_PROPERTIES:
      return "An element must have at least one associated property";
    case ErrorCode::PROPERTY_HAS_NO_INSTANCES:
      return "All elements with properties must have a non-zero number of "
             "instances";
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

using PropertyGenerator = std::variant<
    std::generator<int8_t>, std::generator<std::span<const int8_t>>,
    std::generator<uint8_t>, std::generator<std::span<const uint8_t>>,
    std::generator<int16_t>, std::generator<std::span<const int16_t>>,
    std::generator<uint16_t>, std::generator<std::span<const uint16_t>>,
    std::generator<int32_t>, std::generator<std::span<const int32_t>>,
    std::generator<uint32_t>, std::generator<std::span<const uint32_t>>,
    std::generator<float>, std::generator<std::span<const float>>,
    std::generator<double>, std::generator<std::span<const double>>>;

using GetPropertyListSizeProxy =
    unsigned int (PlyWriter::*)(const std::string&, const std::string&) const;

struct ListTypeParameters {
  std::string_view prefix;
  std::error_code (*serialize_ascii)(std::ostream& output,
                                     std::stringstream& storage, size_t size);
  std::error_code (*serialize_big_endian)(std::ostream& output, size_t size);
  std::error_code (*serialize_little_endian)(std::ostream& output, size_t size);
};

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
                               T value) {
  output << +value;
  if (!output) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::floating_point T>
std::error_code SerializeASCII(std::ostream& output, std::stringstream& storage,
                               T value) {
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
std::error_code SerializeListSizeASCII(std::ostream& output,
                                       std::stringstream& storage,
                                       size_t size) {
  if (std::numeric_limits<T>::max() < size) {
    return ErrorCode::LIST_INDEX_TOO_SMALL;
  }

  return SerializeASCII(output, storage, static_cast<T>(size));
}

template <typename T>
std::error_code SerializeListASCII(std::ostream& output,
                                   std::stringstream& storage,
                                   const ListTypeParameters& parameters,
                                   std::span<const T> values) {
  if (std::error_code error =
          parameters.serialize_ascii(output, storage, values.size());
      error) {
    return error;
  }

  for (const auto& entry : values) {
    if (!output.put(' ')) {
      return std::io_errc::stream;
    }

    if (std::error_code error = SerializeASCII(output, storage, entry); error) {
      return error;
    }
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code SerializeBinary(std::ostream& output, T value) {
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
std::error_code SerializeBinary(std::ostream& output, T value) {
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
std::error_code SerializeListSizeBinary(std::ostream& output, size_t size) {
  if (std::numeric_limits<T>::max() < size) {
    return ErrorCode::LIST_INDEX_TOO_SMALL;
  }

  return SerializeBinary<Endianness>(output, static_cast<T>(size));
}

template <std::endian Endianness, typename T>
std::error_code SerializeListBinary(std::ostream& output,
                                    const ListTypeParameters& parameters,
                                    std::span<const T> values) {
  if constexpr (Endianness == std::endian::big) {
    if (std::error_code error =
            parameters.serialize_big_endian(output, values.size());
        error) {
      return error;
    }
  } else {
    if (std::error_code error =
            parameters.serialize_little_endian(output, values.size());
        error) {
      return error;
    }
  }

  for (const auto& entry : values) {
    if (std::error_code error = SerializeBinary<Endianness>(output, entry);
        error) {
      return error;
    }
  }

  return std::error_code();
}

template <typename T>
std::move_only_function<std::error_code()> MakeASCIISerializer(
    std::ostream& stream, std::stringstream& storage,
    std::generator<T>& generator, const ListTypeParameters* list_parameters) {
  return [&stream, &storage, iter = generator.begin(), end = generator.end(),
          list_parameters]() mutable -> std::error_code {
    if (iter == end) {
      return std::error_code(ErrorCode::NOT_ENOUGH_VALUES);
    }

    if constexpr (std::is_arithmetic<T>::value) {
      if (std::error_code error = SerializeASCII(stream, storage, *iter);
          error) {
        return error;
      }
    } else {
      if (std::error_code error =
              SerializeListASCII(stream, storage, *list_parameters, *iter);
          error) {
        return error;
      }
    }

    iter++;

    return std::error_code();
  };
}

std::move_only_function<std::error_code()> MakeASCIISerializer(
    std::ostream& stream, std::stringstream& storage,
    PropertyGenerator& generator, const ListTypeParameters* list_parameters) {
  return std::visit(
      [&](auto& value) -> std::move_only_function<std::error_code()> {
        return MakeASCIISerializer(stream, storage, value, list_parameters);
      },
      generator);
}

template <std::endian Endianness, typename T>
std::move_only_function<std::error_code()> MakeBinarySerializer(
    std::ostream& stream, std::generator<T>& generator,
    const ListTypeParameters* list_parameters) {
  return [&stream, iter = generator.begin(), end = generator.end(),
          list_parameters]() mutable -> std::error_code {
    if (iter == end) {
      return std::error_code(ErrorCode::NOT_ENOUGH_VALUES);
    }

    if constexpr (std::is_arithmetic<T>::value) {
      if (std::error_code error = SerializeBinary<Endianness>(stream, *iter);
          error) {
        return error;
      }
    } else {
      if (std::error_code error =
              SerializeListBinary<Endianness>(stream, *list_parameters, *iter);
          error) {
        return error;
      }
    }

    iter++;

    return std::error_code();
  };
}

template <std::endian Endianness>
std::move_only_function<std::error_code()> MakeBinarySerializer(
    std::ostream& stream, PropertyGenerator& generator,
    const ListTypeParameters* list_parameters) {
  return std::visit(
      [&](auto& value) -> std::move_only_function<std::error_code()> {
        return MakeBinarySerializer<Endianness>(stream, value, list_parameters);
      },
      generator);
}

static constexpr ListTypeParameters kListParameters[3]{
    {"list uchar ", SerializeListSizeASCII<uint8_t>,
     SerializeListSizeBinary<std::endian::big, uint8_t>,
     SerializeListSizeBinary<std::endian::little, uint8_t>},
    {"list ushort ", SerializeListSizeASCII<uint16_t>,
     SerializeListSizeBinary<std::endian::big, uint16_t>,
     SerializeListSizeBinary<std::endian::little, uint16_t>},
    {"list uint ", SerializeListSizeASCII<uint32_t>,
     SerializeListSizeBinary<std::endian::big, uint32_t>,
     SerializeListSizeBinary<std::endian::little, uint32_t>},
};

std::map<std::string,
         std::map<std::string,
                  std::pair<PropertyGenerator, const ListTypeParameters*>>>
BuildGenerators(std::map<std::string, std::map<std::string, PropertyGenerator>>
                    property_generators,
                const PlyWriter& ply_writer,
                GetPropertyListSizeProxy get_property_list_type) {
  std::map<std::string,
           std::map<std::string,
                    std::pair<PropertyGenerator, const ListTypeParameters*>>>
      result;
  for (auto& [element_name, elements] : property_generators) {
    std::map<std::string,
             std::pair<PropertyGenerator, const ListTypeParameters*>>&
        element_generators = result[element_name];
    for (auto& [property_name, generator] : elements) {
      const ListTypeParameters* name_and_capacity = nullptr;
      if (generator.index() & 1u) {
        unsigned int list_size_type = std::min(
            (ply_writer.*get_property_list_type)(element_name, property_name),
            static_cast<unsigned int>(std::size(kListParameters)));
        name_and_capacity =
            &kListParameters[static_cast<size_t>(list_size_type)];
      }
      element_generators.try_emplace(property_name, std::move(generator),
                                     name_and_capacity);
    }
  }

  return result;
}

std::error_code WriteHeader(
    std::ostream& output, std::string_view format,
    std::map<std::string, uintmax_t>& num_element_instances,
    const std::map<std::string,
                   std::map<std::string, std::pair<PropertyGenerator,
                                                   const ListTypeParameters*>>>&
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

  for (const auto& [element_name, _] : num_element_instances) {
    if (!generators_and_list_details.contains(element_name)) {
      return ErrorCode::ELEMENT_HAS_NO_PROPERTIES;
    }
  }

  for (const auto& [element_name, element_generators] :
       generators_and_list_details) {
    if (std::error_code error = ValidateName(element_name); error) {
      return error;
    }

    uintmax_t num_instances = num_element_instances[element_name];
    if (num_instances == 0) {
      return ErrorCode::PROPERTY_HAS_NO_INSTANCES;
    }

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
          !output.write(generator_and_details.second->prefix.data(),
                        generator_and_details.second->prefix.size())) {
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
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = Start(num_element_instances, property_generators,
                                    comments, object_info);
      error) {
    return error;
  }

  auto generators_with_capacities =
      BuildGenerators(std::move(property_generators), *this,
                      reinterpret_cast<GetPropertyListSizeProxy>(
                          &PlyWriter::GetPropertyListSizeType));

  if (std::error_code error =
          WriteHeader(stream, "ascii", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  std::stringstream storage;
  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      add_next_property.push_back(
          MakeASCIISerializer(stream, storage, generator_and_list_details.first,
                              generator_and_list_details.second));
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
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = Start(num_element_instances, property_generators,
                                    comments, object_info);
      error) {
    return error;
  }

  auto generators_with_capacities =
      BuildGenerators(std::move(property_generators), *this,
                      reinterpret_cast<GetPropertyListSizeProxy>(
                          &PlyWriter::GetPropertyListSizeType));

  if (std::error_code error =
          WriteHeader(stream, "binary_big_endian", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      add_next_property.push_back(MakeBinarySerializer<std::endian::big>(
          stream, generator_and_list_details.first,
          generator_and_list_details.second));
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
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = Start(num_element_instances, property_generators,
                                    comments, object_info);
      error) {
    return error;
  }

  auto generators_with_capacities =
      BuildGenerators(std::move(property_generators), *this,
                      reinterpret_cast<GetPropertyListSizeProxy>(
                          &PlyWriter::GetPropertyListSizeType));

  if (std::error_code error =
          WriteHeader(stream, "binary_little_endian", num_element_instances,
                      generators_with_capacities, comments, object_info)) {
    return error;
  }

  for (auto& [element_name, elements] : generators_with_capacities) {
    std::vector<std::move_only_function<std::error_code()>> add_next_property;
    for (auto& [_, generator_and_list_details] : elements) {
      add_next_property.push_back(MakeBinarySerializer<std::endian::little>(
          stream, generator_and_list_details.first,
          generator_and_list_details.second));
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
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8);
static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
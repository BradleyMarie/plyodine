#include "plyodine/ply_writer.h"

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

enum class ErrorCode {
  MIN_VALUE = 1,
  BAD_STREAM = 1,
  INVALID_COMMENT = 2,
  INVALID_OBJ_INFO = 3,
  MISSING_ELEMENT_NAME = 4,
  INVALID_ELEMENT_NAME = 5,
  MISSING_PROPERTY_NAME = 6,
  INVALID_PROPERTY_NAME = 7,
  OVERFLOWED_UCHAR_LIST = 8,
  OVERFLOWED_USHORT_LIST = 9,
  OVERFLOWED_UINT_LIST = 10,
  MISSING_DATA = 11,
  INVALID_ASCII_FLOAT = 12,
  INVALID_ASCII_FLOAT_LIST = 13,
  MISSING_PROPERTIES = 14,
  MAX_VALUE = 14,
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
      return "The stream was not in 'good' state";
    case ErrorCode::INVALID_COMMENT:
      return "A comment string contained invalid characters (must contain only "
             "printable ASCII characters)";
    case ErrorCode::INVALID_OBJ_INFO:
      return "An obj_info string contained invalid characters (must contain "
             "only printable ASCII characters)";
    case ErrorCode::MISSING_ELEMENT_NAME:
      return "An element had an empty name";
    case ErrorCode::INVALID_ELEMENT_NAME:
      return "An element name contained invalid characters (must contain only "
             "ASCII graphic characters)";
    case ErrorCode::MISSING_PROPERTY_NAME:
      return "A property had an empty name";
    case ErrorCode::INVALID_PROPERTY_NAME:
      return "A property name contained invalid characters (must contain only "
             "ASCII graphic characters)";
    case ErrorCode::OVERFLOWED_UCHAR_LIST:
      return "A property list with size type 'uchar' exceeded its maximum "
             "supported length (255 entries)";
    case ErrorCode::OVERFLOWED_USHORT_LIST:
      return "A property list with size type 'ushort' exceeded its maximum "
             "supported length (65,535 entries)";
    case ErrorCode::OVERFLOWED_UINT_LIST:
      return "A property list with size type 'uint' exceeded its maximum "
             "supported length (4,294,967,295 entries)";
    case ErrorCode::MISSING_DATA:
      return "A property generator did not produce enough data for all "
             "instances of its element";
    case ErrorCode::INVALID_ASCII_FLOAT:
      return "A non-finite floating-point property cannot be written to an "
             "ASCII output";
    case ErrorCode::INVALID_ASCII_FLOAT_LIST:
      return "A non-finite floating-point property list entry cannot be "
             "written to an ASCII output";
    case ErrorCode::MISSING_PROPERTIES:
      return "An element had no properties";
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

enum class Format {
  ASCII = 0,
  BINARY_BIG_ENDIAN = 1,
  BINARY_LITTLE_ENDIAN = 2
};

using GetPropertyListSizeFunc =
    std::function<int(const std::string&, const std::string&)>;
using WriteFunc =
    std::move_only_function<std::error_code(std::ostream&, std::stringstream&)>;

std::error_code ValidateName(const std::string& name, ErrorCode empty_error,
                             ErrorCode invalid_chars_error) {
  if (name.empty()) {
    return empty_error;
  }

  for (char c : name) {
    if (!std::isgraph(c)) {
      return invalid_chars_error;
    }
  }

  return std::error_code();
}

bool ValidateComment(const std::string& comment) {
  for (char c : comment) {
    if (!std::isprint(c)) {
      return false;
    }
  }

  return true;
}

template <std::integral T>
std::error_code SerializeASCII(std::ostream& stream, std::stringstream& storage,
                               T value) {
  stream << +value;
  if (!stream) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::floating_point T>
std::error_code SerializeASCII(std::ostream& stream, std::stringstream& storage,
                               T value) {
  if (!std::isfinite(value)) {
    return ErrorCode::INVALID_ASCII_FLOAT;
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

  if (!stream.write(result.data(), result.size())) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code SerializeBinary(std::ostream& stream,
                                std::stringstream& storage, T value) {
  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  stream.write(reinterpret_cast<char*>(&value), sizeof(value));
  if (!stream) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <std::endian Endianness, std::floating_point T>
std::error_code SerializeBinary(std::ostream& stream,
                                std::stringstream& storage, T value) {
  auto entry = std::bit_cast<
      std::conditional_t<std::is_same_v<T, float>, uint32_t, uintmax_t>>(value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  stream.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!stream) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

template <Format format, typename T>
std::error_code Serialize(std::ostream& stream, std::stringstream& storage,
                          T value) {
  if constexpr (format == Format::ASCII) {
    return SerializeASCII(stream, storage, value);
  } else if constexpr (format == Format::BINARY_BIG_ENDIAN) {
    return SerializeBinary<std::endian::big>(stream, storage, value);
  } else {
    return SerializeBinary<std::endian::little>(stream, storage, value);
  }
}

template <Format F, typename T>
WriteFunc MakeWriteFuncImpl(std::generator<T>& generator, int list_type) {
  static constexpr uint32_t list_capacities[3] = {
      std::numeric_limits<uint8_t>::max(), std::numeric_limits<uint16_t>::max(),
      std::numeric_limits<uint32_t>::max()};
  static constexpr ErrorCode list_errors[3] = {
      ErrorCode::OVERFLOWED_UCHAR_LIST, ErrorCode::OVERFLOWED_USHORT_LIST,
      ErrorCode::OVERFLOWED_UINT_LIST};

  return [iter = generator.begin(), end = generator.end(), list_type](
             std::ostream& stream,
             std::stringstream& token) mutable -> std::error_code {
    if (iter == end) {
      return ErrorCode::MISSING_DATA;
    }

    T value = *iter;

    if constexpr (!std::is_arithmetic_v<T>) {
      size_t size = value.size();
      if (size > list_capacities[static_cast<size_t>(list_type)]) {
        return list_errors[static_cast<size_t>(list_type)];
      }

      switch (list_type) {
        case 0:
          if (std::error_code error =
                  Serialize<F>(stream, token, static_cast<uint8_t>(size));
              error) {
            return error;
          }
          break;
        case 1:
          if (std::error_code error =
                  Serialize<F>(stream, token, static_cast<uint16_t>(size));
              error) {
            return error;
          }
          break;
        case 2:
          if (std::error_code error =
                  Serialize<F>(stream, token, static_cast<uint32_t>(size));
              error) {
            return error;
          }
          break;
      }

      for (size_t i = 0; i < value.size(); i++) {
        if constexpr (F == Format::ASCII) {
          if (!stream.put(' ')) {
            return std::io_errc::stream;
          }
        }

        if (std::error_code error = Serialize<F>(stream, token, value[i]);
            error) {
          if (error == ErrorCode::INVALID_ASCII_FLOAT) {
            return ErrorCode::INVALID_ASCII_FLOAT_LIST;
          }

          return error;
        }
      }
    } else {
      if (std::error_code error = Serialize<F>(stream, token, value); error) {
        return error;
      }
    }

    iter++;

    return std::error_code();
  };
}

template <Format F, typename Variant>
WriteFunc MakeWriteFunc(Variant& generator, int list_type) {
  return std::visit(
      [list_type](auto& gen) { return MakeWriteFuncImpl<F>(gen, list_type); },
      generator);
}

struct Property {
  int list_type;
  int data_type_index;
  WriteFunc write_func;
};

template <Format F, typename T>
std::map<std::string, std::map<std::string, Property>> BuildProperties(
    GetPropertyListSizeFunc get_property_list_size,
    std::map<std::string, std::map<std::string, T>>& generators) {
  std::map<std::string, std::map<std::string, Property>> result;
  for (auto& [element_name, elements] : generators) {
    std::map<std::string, Property>& properties = result[element_name];
    for (auto& [property_name, generator] : elements) {
      int list_type = 2;
      if (generator.index() & 1u) {
        list_type = get_property_list_size(element_name, property_name);
      }
      properties.try_emplace(property_name, list_type, generator.index(),
                             MakeWriteFunc<F>(generator, list_type));
    }
  }

  return result;
}

std::error_code WriteHeader(
    std::ostream& stream, std::string_view format,
    std::map<std::string, uintmax_t>& num_element_instances,
    const std::map<std::string, std::map<std::string, Property>>& elements,
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
  static constexpr std::string_view list_type_prefixes[3] = {
      "list uchar ", "list ushort ", "list uint "};
  static constexpr std::string_view header_suffix = "end_header\r";

  if (!stream.write(header_prefix.data(), header_prefix.size()) ||
      !stream.write(format.data(), format.size()) ||
      !stream.write(version_suffix.data(), version_suffix.size())) {
    return std::io_errc::stream;
  }

  for (const auto& comment : comments) {
    if (!ValidateComment(comment)) {
      return ErrorCode::INVALID_COMMENT;
    }

    if (!stream.write(comment_prefix.data(), comment_prefix.size()) ||
        !stream.write(comment.data(), comment.size()) || !stream.put('\r')) {
      return std::io_errc::stream;
    }
  }

  for (const auto& info : object_info) {
    if (!ValidateComment(info)) {
      return ErrorCode::INVALID_OBJ_INFO;
    }

    if (!stream.write(obj_info_prefix.data(), obj_info_prefix.size()) ||
        !stream.write(info.data(), info.size()) || !stream.put('\r')) {
      return std::io_errc::stream;
    }
  }

  for (const auto& [element_name, properties] : elements) {
    if (properties.empty()) {
      return ErrorCode::MISSING_PROPERTIES;
    }

    if (std::error_code error =
            ValidateName(element_name, ErrorCode::MISSING_ELEMENT_NAME,
                         ErrorCode::INVALID_ELEMENT_NAME);
        error) {
      return error;
    }

    uintmax_t num_instances = num_element_instances[element_name];
    if (!stream.write(element_prefix.data(), element_prefix.size()) ||
        !stream.write(element_name.data(), element_name.size()) ||
        !stream.put(' ') || !(stream << num_instances) || !stream.put('\r')) {
      return std::io_errc::stream;
    }

    for (const auto& [property_name, property] : properties) {
      if (std::error_code error =
              ValidateName(property_name, ErrorCode::MISSING_PROPERTY_NAME,
                           ErrorCode::INVALID_PROPERTY_NAME);
          error) {
        return error;
      }

      if (!stream.write(property_prefix.data(), property_prefix.size())) {
        return std::io_errc::stream;
      }
      if ((property.data_type_index & 1u) &&
          !stream.write(
              list_type_prefixes[static_cast<size_t>(property.list_type)]
                  .data(),
              list_type_prefixes[static_cast<size_t>(property.list_type)]
                  .size())) {
        return std::io_errc::stream;
      }

      const std::string_view& data_type_name =
          data_type_names[property.data_type_index >> 1u];
      if (!stream.write(data_type_name.data(), data_type_name.size()) ||
          !stream.write(property_name.data(), property_name.size()) ||
          !stream.put('\r')) {
        return std::io_errc::stream;
      }
    }
  }

  if (!stream.write(header_suffix.data(), header_suffix.size())) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

std::error_code WriteFile(
    std::ostream& stream, Format format,
    std::map<std::string, uintmax_t>& num_element_instances,
    std::map<std::string, std::map<std::string, Property>>& elements,
    const std::vector<std::string>& comments,
    const std::vector<std::string>& object_info) {
  static constexpr std::string_view format_strings[3] = {
      "ascii", "binary_big_endian", "binary_little_endian"};

  if (std::error_code error =
          WriteHeader(stream, format_strings[static_cast<size_t>(format)],
                      num_element_instances, elements, comments, object_info);
      error) {
    return error;
  }

  std::stringstream storage;
  for (auto& [element_name, properties] : elements) {
    uintmax_t count = num_element_instances[element_name];
    for (uintmax_t i = 0; i < count; i++) {
      bool first = true;
      for (auto& [property_name, property] : properties) {
        if (!first && format == Format::ASCII && !stream.put(' ')) {
          return std::io_errc::stream;
        }

        if (std::error_code error = property.write_func(stream, storage);
            error) {
          return error;
        }

        first = false;
      }

      if (format == Format::ASCII && !stream.put('\r')) {
        return std::io_errc::stream;
      }
    }
  }

  return std::error_code();
}

template <typename T>
GetPropertyListSizeFunc MakeGetPropertyListSizeFunc(
    const PlyWriter& ply_writer,
    T (PlyWriter::*get_property_list_size)(const std::string&,
                                           const std::string&) const) {
  return
      [&ply_writer, get_property_list_size](const std::string& element_name,
                                            const std::string& property_name) {
        int list_type = static_cast<int>(
            (ply_writer.*get_property_list_size)(element_name, property_name));
        if (list_type < 0 || list_type > 2) {
          list_type = 2;
        }

        return list_type;
      };
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
  if (!stream.good()) {
    return ErrorCode::BAD_STREAM;
  }

  std::unique_ptr<const PlyWriter> final_delegate;
  const PlyWriter* ply_writer = this;
  for (;;) {
    std::unique_ptr<const PlyWriter> delegate = ply_writer->DelegateTo();
    if (!delegate) {
      break;
    }

    ply_writer = delegate.get();
    final_delegate = std::move(delegate);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = ply_writer->Start(
          num_element_instances, property_generators, comments, object_info);
      error) {
    return error;
  }

  auto properties = BuildProperties<Format::ASCII>(
      MakeGetPropertyListSizeFunc(*ply_writer,
                                  &PlyWriter::GetPropertyListSizeType),
      property_generators);

  return WriteFile(stream, Format::ASCII, num_element_instances, properties,
                   comments, object_info);
}

std::error_code PlyWriter::WriteToBigEndian(std::ostream& stream) const {
  if (!stream.good()) {
    return ErrorCode::BAD_STREAM;
  }

  std::unique_ptr<const PlyWriter> final_delegate;
  const PlyWriter* ply_writer = this;
  for (;;) {
    std::unique_ptr<const PlyWriter> delegate = ply_writer->DelegateTo();
    if (!delegate) {
      break;
    }

    ply_writer = delegate.get();
    final_delegate = std::move(delegate);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = ply_writer->Start(
          num_element_instances, property_generators, comments, object_info);
      error) {
    return error;
  }

  auto properties = BuildProperties<Format::BINARY_BIG_ENDIAN>(
      MakeGetPropertyListSizeFunc(*ply_writer,
                                  &PlyWriter::GetPropertyListSizeType),
      property_generators);

  return WriteFile(stream, Format::BINARY_BIG_ENDIAN, num_element_instances,
                   properties, comments, object_info);
}

std::error_code PlyWriter::WriteToLittleEndian(std::ostream& stream) const {
  if (!stream.good()) {
    return ErrorCode::BAD_STREAM;
  }

  std::unique_ptr<const PlyWriter> final_delegate;
  const PlyWriter* ply_writer = this;
  for (;;) {
    std::unique_ptr<const PlyWriter> delegate = ply_writer->DelegateTo();
    if (!delegate) {
      break;
    }

    ply_writer = delegate.get();
    final_delegate = std::move(delegate);
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, PropertyGenerator>>
      property_generators;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  if (std::error_code error = ply_writer->Start(
          num_element_instances, property_generators, comments, object_info);
      error) {
    return error;
  }

  auto properties = BuildProperties<Format::BINARY_LITTLE_ENDIAN>(
      MakeGetPropertyListSizeFunc(*ply_writer,
                                  &PlyWriter::GetPropertyListSizeType),
      property_generators);

  return WriteFile(stream, Format::BINARY_LITTLE_ENDIAN, num_element_instances,
                   properties, comments, object_info);
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8);
static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
#include "plyodine/ply_header_reader.h"

#include <bit>
#include <charconv>
#include <cstdint>
#include <expected>
#include <ios>
#include <istream>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

enum class ErrorCode {
  MIN_VALUE = 1,
  BAD_STREAM = 1,
  MISSING_MAGIC_WORD = 2,
  MISMATCHED_LINE_ENDINGS = 3,
  CONTAINS_INVALID_CHARACTER = 4,
  MISSING_FORMAT_SPECIFIER = 5,
  SPECIFIED_INVALID_FORMAT = 6,
  SPECIFIED_UNSUPPORTED_VERSION = 7,
  FORMAT_SPECIFIER_TOO_LONG = 8,
  NAKED_PROPERTY = 9,
  PROPERTY_SPECIFIER_TOO_SHORT = 10,
  PROPERTY_SPECIFIED_INVALID_TYPE = 11,
  PROPERTY_SPECIFIED_LIST_TYPE_FLOAT = 12,
  PROPERTY_SPECIFIED_LIST_TYPE_DOUBLE = 13,
  PROPERTY_SPECIFIED_DUPLICATE_NAME = 14,
  PROPERTY_SPECIFIER_TOO_LONG = 15,
  ELEMENT_SPECIFIER_TOO_SHORT = 16,
  ELEMENT_SPECIFIED_DUPLICATE_NAME = 17,
  ELEMENT_COUNT_OUT_OF_RANGE = 18,
  ELEMENT_COUNT_PARSING_FAILED = 19,
  ELEMENT_SPECIFIER_TOO_LONG = 20,
  END_INVALID = 21,
  UNRECOGNIZED_KEYWORD = 22,
  MAX_VALUE = 22,
};

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
  std::error_condition default_error_condition(
      int value) const noexcept override;
} kErrorCategory;

const char* ErrorCategory::name() const noexcept {
  return "plyodine::ReadPlyHeader";
}

std::string ErrorCategory::message(int condition) const {
  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "The stream was not in 'good' state";
    case ErrorCode::MISSING_MAGIC_WORD:
      return "The first line of the input must contain only the word 'ply'";
    case ErrorCode::MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::CONTAINS_INVALID_CHARACTER:
      return "The input contained an invalid character";
    case ErrorCode::MISSING_FORMAT_SPECIFIER:
      return "The second line of the input must contain the format specifier";
    case ErrorCode::SPECIFIED_INVALID_FORMAT:
      return "Format must be one of ascii, binary_big_endian, or "
             "binary_little_endian";
    case ErrorCode::SPECIFIED_UNSUPPORTED_VERSION:
      return "Only PLY version 1.0 supported";
    case ErrorCode::FORMAT_SPECIFIER_TOO_LONG:
      return "The format specifier contained too many parameters";
    case ErrorCode::NAKED_PROPERTY:
      return "A property could not be associated with an element";
    case ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT:
      return "A property specifier contained too few parameters";
    case ErrorCode::PROPERTY_SPECIFIED_INVALID_TYPE:
      return "A property is of an invalid type";
    case ErrorCode::PROPERTY_SPECIFIED_LIST_TYPE_FLOAT:
      return "A property list cannot have float as its list type";
    case ErrorCode::PROPERTY_SPECIFIED_LIST_TYPE_DOUBLE:
      return "A property list cannot have double as its list type";
    case ErrorCode::PROPERTY_SPECIFIED_DUPLICATE_NAME:
      return "An element contains two properties with the same name";
    case ErrorCode::PROPERTY_SPECIFIER_TOO_LONG:
      return "Too many parameters to property";
    case ErrorCode::ELEMENT_SPECIFIER_TOO_SHORT:
      return "Too few parameters to element";
    case ErrorCode::ELEMENT_SPECIFIED_DUPLICATE_NAME:
      return "Two elements have the same name";
    case ErrorCode::ELEMENT_COUNT_OUT_OF_RANGE:
      return "Out of range element count";
    case ErrorCode::ELEMENT_COUNT_PARSING_FAILED:
      return "Failed to parse element count";
    case ErrorCode::ELEMENT_SPECIFIER_TOO_LONG:
      return "Too many parameters to element";
    case ErrorCode::END_INVALID:
      return "The last line of the header may only contain the end_header "
             "keyword";
    case ErrorCode::UNRECOGNIZED_KEYWORD:
      return "The input contained an invalid header";
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

std::expected<std::string_view, std::error_code> ReadNextLine(
    std::istream& input, std::string& storage, std::string_view line_ending) {
  storage.clear();

  char c;
  while (input.get(c)) {
    if (c == '\r' || c == '\n') {
      do {
        if (c != line_ending[0]) {
          return std::unexpected(ErrorCode::MISMATCHED_LINE_ENDINGS);
        }
        line_ending.remove_prefix(1);
      } while (!line_ending.empty() && input.get(c));
      break;
    }

    if (c == '\r' || c == '\n') {
      return std::unexpected(ErrorCode::MISMATCHED_LINE_ENDINGS);
    }

    if (c == '\t') {
      c = ' ';
    }

    if (!std::isprint(c)) {
      return std::unexpected(ErrorCode::CONTAINS_INVALID_CHARACTER);
    }

    storage.push_back(c);
  }

  if (input.fail() && !input.eof()) {
    return std::unexpected(std::io_errc::stream);
  }

  return storage;
}

std::optional<std::string_view> ReadNextToken(std::string_view& line) {
  size_t prefix_length = line.find_first_not_of(' ');
  if (prefix_length == std::string_view::npos) {
    return std::nullopt;
  }

  line.remove_prefix(prefix_length);

  size_t token_length = line.find(' ');
  if (token_length == std::string::npos) {
    token_length = line.length();
  }

  std::string_view result = line.substr(0u, token_length);
  line.remove_prefix(token_length);

  return result;
}

std::expected<std::string, std::error_code> ParseMagicString(
    std::istream& input) {
  char c = 0;
  while (input.get(c)) {
    if (c != ' ' && c != '\t') {
      break;
    }
  };

  if (c != 'p' || !input.get(c) || c != 'l' || !input.get(c) || c != 'y') {
    return std::unexpected(ErrorCode::MISSING_MAGIC_WORD);
  }

  c = 0;
  while (input.get(c)) {
    if (c != ' ' && c != '\t') {
      break;
    }
  };

  if (c != '\r' && c != '\n') {
    return std::unexpected(ErrorCode::MISSING_MAGIC_WORD);
  }

  // The original documentation describing the PLY format mandates the use of
  // carriage return for all ASCII line endings; however, this requirement seems
  // to have been lost to time and it is common to find PLY files with any of
  // the three major line endings. Plyodine therefor supports all three of these
  // line endings only requiring that parsed files are consistent throughout.
  if (c == '\n') {
    return "\n";
  }

  if (input.peek() == '\n') {
    input.get();
    return "\r\n";
  }

  return "\r";
}

bool CheckVersion(std::string_view version) {
  size_t prefix_length = version.find_first_not_of('0');
  if (prefix_length == std::string_view::npos) {
    return false;
  }

  version.remove_prefix(prefix_length);

  if (version[0] != '1') {
    return false;
  }

  version.remove_prefix(1);

  if (version.empty()) {
    return true;
  }

  if (version[0] != '.') {
    return false;
  }

  version.remove_prefix(1);

  if (version.empty()) {
    return true;
  }

  for (char c : version) {
    if (c != '0') {
      return false;
    }
  }

  return true;
}

std::expected<PlyHeader::Format, std::error_code> ParseFormat(
    std::istream& input, std::string& storage, const std::string& line_ending) {
  auto line = ReadNextLine(input, storage, line_ending);
  if (!line) {
    return std::unexpected(line.error());
  }

  if (std::optional<std::string_view> token = ReadNextToken(*line);
      !token || *token != "format") {
    return std::unexpected(ErrorCode::MISSING_FORMAT_SPECIFIER);
  }

  PlyHeader::Format format;
  if (std::optional<std::string_view> token = ReadNextToken(*line); token) {
    if (*token == "ascii") {
      format = PlyHeader::Format::ASCII;
    } else if (*token == "binary_big_endian") {
      format = PlyHeader::Format::BINARY_BIG_ENDIAN;
    } else if (*token == "binary_little_endian") {
      format = PlyHeader::Format::BINARY_LITTLE_ENDIAN;
    } else {
      return std::unexpected(ErrorCode::SPECIFIED_INVALID_FORMAT);
    }
  } else {
    return std::unexpected(ErrorCode::SPECIFIED_INVALID_FORMAT);
  }

  if (std::optional<std::string_view> token = ReadNextToken(*line);
      !token || !CheckVersion(*token)) {
    return std::unexpected(ErrorCode::SPECIFIED_UNSUPPORTED_VERSION);
  }

  if (std::optional<std::string_view> token = ReadNextToken(*line); token) {
    return std::unexpected(ErrorCode::FORMAT_SPECIFIER_TOO_LONG);
  }

  return format;
}

std::expected<std::pair<std::string, uintmax_t>, std::error_code> ParseElement(
    std::string_view line,
    const std::unordered_set<std::string>& element_names) {
  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::ELEMENT_SPECIFIER_TOO_SHORT);
  }

  std::string str_name(*name);
  if (element_names.contains(str_name)) {
    return std::unexpected(ErrorCode::ELEMENT_SPECIFIED_DUPLICATE_NAME);
  }

  std::optional<std::string_view> num_in_file = ReadNextToken(line);
  if (!num_in_file) {
    return std::unexpected(ErrorCode::ELEMENT_SPECIFIER_TOO_SHORT);
  }

  uintmax_t parsed_num_in_file;
  auto parsing_result = std::from_chars(
      num_in_file->data(), num_in_file->data() + num_in_file->size(),
      parsed_num_in_file);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return std::unexpected(ErrorCode::ELEMENT_COUNT_OUT_OF_RANGE);
  } else if (parsing_result.ec != std::errc{}) {
    return std::unexpected(ErrorCode::ELEMENT_COUNT_PARSING_FAILED);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::ELEMENT_SPECIFIER_TOO_LONG);
  }

  return std::make_pair(std::move(str_name), parsed_num_in_file);
}

std::expected<PlyHeader::Property::Type, std::error_code> ParseType(
    std::string_view type_name) {
  static const std::unordered_map<std::string_view, PlyHeader::Property::Type>
      type_map = {{"char", PlyHeader::Property::Type::CHAR},
                  {"uchar", PlyHeader::Property::Type::UCHAR},
                  {"int8", PlyHeader::Property::Type::CHAR},
                  {"uint8", PlyHeader::Property::Type::UCHAR},
                  {"short", PlyHeader::Property::Type::SHORT},
                  {"ushort", PlyHeader::Property::Type::USHORT},
                  {"int16", PlyHeader::Property::Type::SHORT},
                  {"uint16", PlyHeader::Property::Type::USHORT},
                  {"int", PlyHeader::Property::Type::INT},
                  {"uint", PlyHeader::Property::Type::UINT},
                  {"int32", PlyHeader::Property::Type::INT},
                  {"uint32", PlyHeader::Property::Type::UINT},
                  {"float", PlyHeader::Property::Type::FLOAT},
                  {"float32", PlyHeader::Property::Type::FLOAT},
                  {"double", PlyHeader::Property::Type::DOUBLE},
                  {"float64", PlyHeader::Property::Type::DOUBLE}};
  auto iter = type_map.find(type_name);
  if (iter == type_map.end()) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIED_INVALID_TYPE);
  }

  return iter->second;
}

std::expected<PlyHeader::Property, std::error_code> ParsePropertyList(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  std::optional<std::string_view> first_token = ReadNextToken(line);
  if (!first_token) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT);
  }

  auto list_type = ParseType(*first_token);
  if (!list_type) {
    return std::unexpected(list_type.error());
  }

  if (*list_type == PlyHeader::Property::Type::FLOAT) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIED_LIST_TYPE_FLOAT);
  }

  if (*list_type == PlyHeader::Property::Type::DOUBLE) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIED_LIST_TYPE_DOUBLE);
  }

  std::optional<std::string_view> second_token = ReadNextToken(line);
  if (!second_token) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT);
  }

  auto data_type = ParseType(*second_token);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT);
  }

  std::string str_name(*name);
  if (property_names.contains(str_name)) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIED_DUPLICATE_NAME);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_LONG);
  }

  return PlyHeader::Property{std::move(str_name), *data_type, *list_type};
}

std::expected<PlyHeader::Property, std::error_code> ParseProperty(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  std::optional<std::string_view> first_token = ReadNextToken(line);
  if (!first_token) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT);
  }

  if (*first_token == "list") {
    return ParsePropertyList(line, property_names);
  }

  auto data_type = ParseType(*first_token);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_SHORT);
  }

  std::string str_name(*name);
  if (property_names.contains(str_name)) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIED_DUPLICATE_NAME);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::PROPERTY_SPECIFIER_TOO_LONG);
  }

  return PlyHeader::Property{std::move(str_name), *data_type};
}

}  // namespace

std::expected<PlyHeader, std::error_code> ReadPlyHeader(std::istream& stream) {
  if (!stream) {
    return std::unexpected(ErrorCode::BAD_STREAM);
  }

  auto line_ending = ParseMagicString(stream);
  if (!line_ending) {
    return std::unexpected(line_ending.error());
  }

  std::string storage;
  auto format = ParseFormat(stream, storage, *line_ending);
  if (!format) {
    return std::unexpected(format.error());
  }

  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  std::vector<PlyHeader::Element> elements;
  std::unordered_set<std::string> element_names;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      property_names;
  for (;;) {
    auto line = ReadNextLine(stream, storage, *line_ending);
    if (!line) {
      return std::unexpected(line.error());
    }

    std::optional<std::string_view> first_token = ReadNextToken(*line);
    if (first_token) {
      if (*first_token == "property") {
        if (elements.empty()) {
          return std::unexpected(ErrorCode::NAKED_PROPERTY);
        }

        auto& element_property_names = property_names[elements.back().name];
        auto property = ParseProperty(*line, element_property_names);
        if (!property) {
          return std::unexpected(property.error());
        }

        elements.back().properties.push_back(*property);
        element_property_names.insert(property->name);
        continue;
      } else if (*first_token == "element") {
        auto element = ParseElement(*line, element_names);
        if (!element) {
          return std::unexpected(element.error());
        }

        element_names.insert(element->first);
        elements.emplace_back(element->first, element->second);
        continue;
      } else if (*first_token == "comment") {
        if (!line->empty()) {
          line->remove_prefix(1);
        }

        comments.emplace_back(*line);
        continue;
      } else if (*first_token == "obj_info") {
        if (!line->empty()) {
          line->remove_prefix(1);
        }

        object_info.emplace_back(*line);
        continue;
      } else if (*first_token == "end_header") {
        if (std::optional<std::string_view> next_token = ReadNextToken(*line);
            next_token) {
          return std::unexpected(ErrorCode::END_INVALID);
        }

        break;
      }
    }

    return std::unexpected(ErrorCode::UNRECOGNIZED_KEYWORD);
  }

  return PlyHeader{*format,
                   *line_ending,
                   1u,
                   0u,
                   std::move(comments),
                   std::move(object_info),
                   std::move(elements)};
}

}  // namespace plyodine
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
  INVALID_MAGIC_WORD = 2,
  MISMATCHED_LINE_ENDINGS = 3,
  INVALID_CHARACTER = 4,
  INVALID_FORMAT_SPECIFIER = 5,
  INVALID_FORMAT = 6,
  UNSUPPORTED_VERSION = 7,
  UNBOUND_PROPERTY = 8,
  INVALID_PROPERTY_OR_LIST = 9,
  INVALID_PROPERTY = 10,
  INVALID_PROPERTY_TYPE = 11,
  INVALID_PROPERTY_LIST = 12,
  INVALID_PROPERTY_LIST_SIZE_TYPE = 13,
  INVALID_PROPERTY_LIST_FLOAT = 14,
  INVALID_PROPERTY_LIST_DOUBLE = 15,
  INVALID_PROPERTY_LIST_DATA_TYPE = 16,
  INVALID_PROPERTY_DUPLICATE_NAME = 17,
  INVALID_ELEMENT = 18,
  INVALID_ELEMENT_DUPLICATE_NAME = 19,
  ELEMENT_COUNT_FAILED_TO_PARSE = 20,
  ELEMENT_COUNT_OUT_OF_RANGE = 21,
  INVALID_HEADER_END = 22,
  UNRECOGNIZED_KEYWORD = 23,
  EMPTY_LINE = 24,
  MAX_VALUE = 24,
};

std::string UIntMaxMax() {
  std::string without_commas =
      std::to_string(std::numeric_limits<uintmax_t>::max());

  std::string result;
  for (size_t i = 0; i < without_commas.size(); i++) {
    size_t index = without_commas.size() - i - 1;
    result = without_commas[index] + result;
    if (i % 3 == 2 && index != 0) {
      result = ',' + result;
    }
  }

  return result;
}

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
  static const std::string max_element_count = UIntMaxMax();

  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "The stream was not in 'good' state";
    case ErrorCode::INVALID_MAGIC_WORD:
      return "The input must contain only 'ply' on its first line";
    case ErrorCode::MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::INVALID_CHARACTER:
      return "The input contained an invalid character in its header (each "
             "line must contain only printable ASCII characters)";
    case ErrorCode::INVALID_FORMAT_SPECIFIER:
      return "The input must contain only the format specifier on its second "
             "line (must have structure 'format "
             "<ascii|binary_big_endian|binary_little_endian> 1.0')";
    case ErrorCode::INVALID_FORMAT:
      return "The input specified an invalid format (must be one of 'ascii', "
             "'binary_big_endian', or 'binary_little_endian')";
    case ErrorCode::UNSUPPORTED_VERSION:
      return "The input specified an unsupported PLY version (maximum "
             "supported version is '1.0')";
    case ErrorCode::UNBOUND_PROPERTY:
      return "The input declared a property before its first element "
             "declaration";
    case ErrorCode::INVALID_PROPERTY_OR_LIST:
      return "The input contained an invalid property declaration (its line "
             "must have structure 'property [(list "
             "<char|uchar|short|ushort|int|uint>)] "
             "<char|uchar|short|ushort|int|uint|float|double> <name>')";
    case ErrorCode::INVALID_PROPERTY:
      return "The input contained an invalid property declaration (its line "
             "must have structure 'property "
             "<char|uchar|short|ushort|int|uint|float|double> <name>')";
    case ErrorCode::INVALID_PROPERTY_TYPE:
      return "The input contained a property declaration with an invalid type "
             "(must be one of 'char', 'uchar', 'short', 'ushort', 'int', "
             "'uint', 'float', or 'double')";
    case ErrorCode::INVALID_PROPERTY_LIST:
      return "The input contained an invalid property list declaration (its "
             "line must have structure 'property list "
             "<char|uchar|short|ushort|int|uint> "
             "<char|uchar|short|ushort|int|uint|float|double> <name>')";
    case ErrorCode::INVALID_PROPERTY_LIST_SIZE_TYPE:
      return "The input contained a property list declaration with an invalid "
             "size type (must be one of 'char', 'uchar', 'short', 'ushort', "
             "'int', or 'uint')";
    case ErrorCode::INVALID_PROPERTY_LIST_FLOAT:
      return "The input contained a property list declaration that specified "
             "'float' as its size type (must be one of 'char', 'uchar', "
             "'short', 'ushort', 'int', or 'uint')";
    case ErrorCode::INVALID_PROPERTY_LIST_DOUBLE:
      return "The input contained a property list declaration that specified "
             "'double' as its size type (must be one of 'char', 'uchar', "
             "'short', 'ushort', 'int', or 'uint')";
    case ErrorCode::INVALID_PROPERTY_LIST_DATA_TYPE:
      return "The input contained a property list declaration with an invalid "
             "data type (must be one of 'char', 'uchar', 'short', 'ushort', "
             "'int', 'uint', 'float', or 'double')";
    case ErrorCode::INVALID_PROPERTY_DUPLICATE_NAME:
      return "The input declared two properties of an element with the same "
             "name";
    case ErrorCode::INVALID_ELEMENT:
      return "The input contained an invalid element declaration (its line "
             "must have structure 'element <name> <number of instances>')";
    case ErrorCode::INVALID_ELEMENT_DUPLICATE_NAME:
      return "The input declared two elements with the same name";
    case ErrorCode::ELEMENT_COUNT_FAILED_TO_PARSE:
      return "The input contained an element declaration with an instance "
             "count that could not be parsed as an integer";
    case ErrorCode::ELEMENT_COUNT_OUT_OF_RANGE:
      return "The input contained an element declaration with an instance "
             "count that was out of range (must be an integer between 0 and " +
             max_element_count + ")";
    case ErrorCode::INVALID_HEADER_END:
      return "The input contained an invalid header sentinel (its line may "
             "contain only 'end_header')";
    case ErrorCode::UNRECOGNIZED_KEYWORD:
      return "The input contained an invalid keyword in its header";
    case ErrorCode::EMPTY_LINE:
      return "The input contained an empty line in its header";
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
    std::istream& stream, std::string& storage, std::string_view line_ending) {
  storage.clear();

  char c;
  while (stream.get(c)) {
    if (c == line_ending[0]) {
      line_ending.remove_prefix(1);

      while (!line_ending.empty()) {
        if (!stream.get(c) || c != line_ending[0]) {
          return std::unexpected(ErrorCode::MISMATCHED_LINE_ENDINGS);
        }

        line_ending.remove_prefix(1);
      }

      break;
    }

    if (c == '\r' || c == '\n') {
      return std::unexpected(ErrorCode::MISMATCHED_LINE_ENDINGS);
    }

    if (c == '\t') {
      c = ' ';
    }

    if (!std::isprint(c)) {
      return std::unexpected(ErrorCode::INVALID_CHARACTER);
    }

    storage.push_back(c);
  }

  if (stream.fail() && !stream.eof()) {
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
    std::istream& stream) {
  char c = 0;
  while (stream.get(c)) {
    if (c != ' ' && c != '\t') {
      break;
    }
  };

  if (c != 'p' || !stream.get(c) || c != 'l' || !stream.get(c) || c != 'y') {
    return std::unexpected(ErrorCode::INVALID_MAGIC_WORD);
  }

  c = 0;
  while (stream.get(c)) {
    if (c != ' ' && c != '\t') {
      break;
    }
  };

  if (c != '\r' && c != '\n') {
    return std::unexpected(ErrorCode::INVALID_MAGIC_WORD);
  }

  // The original documentation describing the PLY format mandates the use of
  // carriage return for all ASCII line endings; however, this requirement seems
  // to have been lost to time and it is common to find PLY files with any of
  // the three major line endings. Plyodine therefor supports all three of these
  // line endings only requiring that parsed files are consistent throughout.
  if (c == '\n') {
    return "\n";
  }

  if (stream.peek() == '\n') {
    stream.get();
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
    std::istream& stream, std::string& storage,
    const std::string& line_ending) {
  auto line = ReadNextLine(stream, storage, line_ending);
  if (!line) {
    return std::unexpected(line.error());
  }

  if (std::optional<std::string_view> token = ReadNextToken(*line);
      !token || *token != "format") {
    return std::unexpected(ErrorCode::INVALID_FORMAT_SPECIFIER);
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
      return std::unexpected(ErrorCode::INVALID_FORMAT);
    }
  } else {
    return std::unexpected(ErrorCode::INVALID_FORMAT_SPECIFIER);
  }

  std::optional<std::string_view> version_token = ReadNextToken(*line);
  if (!version_token) {
    return std::unexpected(ErrorCode::INVALID_FORMAT_SPECIFIER);
  }

  if (!CheckVersion(*version_token)) {
    return std::unexpected(ErrorCode::UNSUPPORTED_VERSION);
  }

  if (std::optional<std::string_view> token = ReadNextToken(*line); token) {
    return std::unexpected(ErrorCode::INVALID_FORMAT_SPECIFIER);
  }

  return format;
}

std::expected<std::pair<std::string, uintmax_t>, std::error_code> ParseElement(
    std::string_view line,
    const std::unordered_set<std::string>& element_names) {
  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::INVALID_ELEMENT);
  }

  std::string str_name(*name);
  if (element_names.contains(str_name)) {
    return std::unexpected(ErrorCode::INVALID_ELEMENT_DUPLICATE_NAME);
  }

  std::optional<std::string_view> instance_count = ReadNextToken(line);
  if (!instance_count) {
    return std::unexpected(ErrorCode::INVALID_ELEMENT);
  }

  const char* start = instance_count->data();
  const char* end = instance_count->data() + instance_count->size();

  bool out_of_range = start[0] == '-';
  if (out_of_range) {
    start += 1;
  }

  uintmax_t parsed_instance_count;
  std::from_chars_result result =
      std::from_chars(start, end, parsed_instance_count);
  if (result.ec == std::errc::invalid_argument || result.ptr != end) {
    return std::unexpected(ErrorCode::ELEMENT_COUNT_FAILED_TO_PARSE);
  } else if (result.ec == std::errc::result_out_of_range || out_of_range) {
    return std::unexpected(ErrorCode::ELEMENT_COUNT_OUT_OF_RANGE);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::INVALID_ELEMENT);
  }

  return std::make_pair(std::move(str_name), parsed_instance_count);
}

std::expected<PlyHeader::Property::Type, std::error_code> ParseType(
    std::string_view type_name, ErrorCode invalid_type_error) {
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
    return std::unexpected(invalid_type_error);
  }

  return iter->second;
}

std::expected<PlyHeader::Property, std::error_code> ParsePropertyList(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  std::optional<std::string_view> first_token = ReadNextToken(line);
  if (!first_token) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST);
  }

  auto list_type =
      ParseType(*first_token, ErrorCode::INVALID_PROPERTY_LIST_SIZE_TYPE);
  if (!list_type) {
    return std::unexpected(list_type.error());
  }

  if (*list_type == PlyHeader::Property::Type::FLOAT) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST_FLOAT);
  }

  if (*list_type == PlyHeader::Property::Type::DOUBLE) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST_DOUBLE);
  }

  std::optional<std::string_view> second_token = ReadNextToken(line);
  if (!second_token) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST);
  }

  auto data_type =
      ParseType(*second_token, ErrorCode::INVALID_PROPERTY_LIST_DATA_TYPE);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST);
  }

  std::string str_name(*name);
  if (property_names.contains(str_name)) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_DUPLICATE_NAME);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_LIST);
  }

  return PlyHeader::Property{std::move(str_name), *data_type, *list_type};
}

std::expected<PlyHeader::Property, std::error_code> ParseProperty(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  std::optional<std::string_view> first_token = ReadNextToken(line);
  if (!first_token) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_OR_LIST);
  }

  if (*first_token == "list") {
    return ParsePropertyList(line, property_names);
  }

  auto data_type = ParseType(*first_token, ErrorCode::INVALID_PROPERTY_TYPE);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  std::optional<std::string_view> name = ReadNextToken(line);
  if (!name) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY);
  }

  std::string str_name(*name);
  if (property_names.contains(str_name)) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY_DUPLICATE_NAME);
  }

  if (std::optional<std::string_view> token = ReadNextToken(line); token) {
    return std::unexpected(ErrorCode::INVALID_PROPERTY);
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
          return std::unexpected(ErrorCode::UNBOUND_PROPERTY);
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
          return std::unexpected(ErrorCode::INVALID_HEADER_END);
        }

        break;
      }

      return std::unexpected(ErrorCode::UNRECOGNIZED_KEYWORD);
    }

    return std::unexpected(ErrorCode::EMPTY_LINE);
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
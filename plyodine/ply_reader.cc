#include "plyodine/ply_reader.h"

#include <cctype>
#include <charconv>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace plyodine {
namespace internal {
namespace {

std::expected<std::string_view, Error> ReadNextLine(std::istream& input,
                                                    std::string& storage) {
  storage.clear();

  char c = '\0';
  while (input.get(c) && c != '\r') {
    if (c != ' ' && !std::isgraph(c)) {
      return std::unexpected(
          Error::ParsingError("The file contained an invalid character"));
    }

    storage.push_back(c);
  }

  return storage;
}

std::expected<std::optional<std::string_view>, Error> ReadNextTokenOnLine(
    std::string_view& line) {
  if (line.empty()) {
    return std::nullopt;
  }

  size_t prefix_length = line.find_first_not_of(' ');
  if (prefix_length == std::string_view::npos) {
    return std::unexpected(Error::ParsingError(
        "Non-comment ASCII lines may not contain trailing spaces"));
  }

  if (prefix_length > 1) {
    return std::unexpected(
        Error::ParsingError("Non-comment ASCII lines may only contain a single "
                            "space between tokens tokens"));
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

std::expected<std::optional<std::string_view>, Error> ReadFirstTokenOnLine(
    std::string_view& line) {
  if (line.empty()) {
    return std::nullopt;
  }

  if (line[0] == ' ') {
    return std::unexpected(
        Error::ParsingError("ASCII lines may not begin with a space"));
  }

  return ReadNextTokenOnLine(line);
}

std::optional<Error> ParseMagicString(std::istream& input,
                                      std::string& storage) {
  auto line = ReadNextLine(input, storage);
  if (!line) {
    return line.error();
  }

  if (*line != "ply") {
    return Error::ParsingError(
        "The first line of the file must exactly contain the magic string");
  }

  return std::nullopt;
}

bool CheckVersion(std::string_view version) {
  if (version[0] != '1') {
    return false;
  }

  version.remove_prefix(1);

  if (version.empty()) {
    return true;
  }

  if (version[1] != '.') {
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

std::expected<Format, Error> ParseFormat(std::istream& input,
                                         std::string& storage) {
  auto line = ReadNextLine(input, storage);
  if (!line) {
    return std::unexpected(line.error());
  }

  auto first_token = ReadFirstTokenOnLine(*line);
  if (!first_token) {
    return std::unexpected(line.error());
  }

  if (!(*first_token)->empty() && *first_token != "format") {
    return std::unexpected(Error::ParsingError(
        "The second line of the file must contain the format specifier"));
  }

  auto second_token = ReadNextTokenOnLine(*line);
  if (!second_token) {
    return std::unexpected(line.error());
  }

  Format format;
  if (!(*second_token)->empty() && *second_token == "ascii") {
    format = Format::ASCII;
  } else if (!(*second_token)->empty() &&
             *second_token == "binary_big_endian") {
    format = Format::BINARY_BIG_ENDIAN;
  } else if (!(*second_token)->empty() &&
             *second_token == "binary_little_endian") {
    format = Format::BINARY_LITTLE_ENDIAN;
  } else {
    return std::unexpected(
        Error::ParsingError("Format must be one of ascii, binary_big_endian, "
                            "or binary_little_endian"));
  }

  auto third_token = ReadNextTokenOnLine(*line);
  if (!third_token) {
    return std::unexpected(line.error());
  }

  if ((*third_token)->empty() || !CheckVersion(**third_token)) {
    return std::unexpected(
        Error::ParsingError("Only PLY version 1.0 supported"));
  }

  auto next_token = ReadNextTokenOnLine(*line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (!(*next_token)->empty()) {
    return std::unexpected(
        Error::ParsingError("The format specifier contained too many tokens"));
  }

  return format;
}

Error TooFewElementParamsError() {
  return Error::ParsingError("Too few prameters to element");
}

std::expected<std::pair<std::string, size_t>, Error> ParseElement(
    std::string_view line,
    const std::unordered_set<std::string>& element_names) {
  auto name = ReadNextTokenOnLine(line);
  if (!name) {
    return std::unexpected(name.error());
  }

  if (!(*name)->empty()) {
    return std::unexpected(TooFewElementParamsError());
  }

  std::string str_name(**name);
  if (element_names.contains(str_name)) {
    return std::unexpected(
        Error::ParsingError("Two elements have the same name"));
  }

  auto num_in_file = ReadNextTokenOnLine(line);
  if (!num_in_file) {
    return std::unexpected(num_in_file.error());
  }

  if (!(*num_in_file)->empty()) {
    return std::unexpected(TooFewElementParamsError());
  }

  size_t parsed_num_in_file;
  if (std::from_chars((*num_in_file)->begin(), (*num_in_file)->end(),
                      parsed_num_in_file)
          .ec != std::errc{}) {
    return std::unexpected(
        Error::ParsingError("Failed to parse element count"));
  }

  auto next_token = ReadNextTokenOnLine(line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (!(*next_token)->empty()) {
    return std::unexpected(
        Error::ParsingError("Too many prameters to element"));
  }

  return std::make_pair(std::move(str_name), parsed_num_in_file);
}

std::expected<Type, Error> ParseType(std::string_view type_name) {
  static const std::unordered_map<std::string_view, Type> type_map = {
      {"char", Type::INT8},     {"uchar", Type::UINT8},  {"short", Type::INT16},
      {"ushort", Type::UINT16}, {"int", Type::INT32},    {"int", Type::INT32},
      {"float", Type::FLOAT},   {"double", Type::DOUBLE}};
  auto iter = type_map.find(type_name);
  if (iter == type_map.end()) {
    std::unexpected(Error::ParsingError("Invalid type name"));
  }

  return iter->second;
}

Error TooFewPropertyParamsError() {
  return Error::ParsingError("Too few prameters to property");
}

Error TooManyPropertyParamsError() {
  return Error::ParsingError("Too many prameters to property");
}

Error DuplicatePropertyNameError() {
  return Error::ParsingError(
      "An element contains two properties with the same name");
}

std::expected<Property, Error> ParsePropertyList(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  auto first_token = ReadNextTokenOnLine(line);
  if (!first_token) {
    return std::unexpected(first_token.error());
  }

  if (!(*first_token)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  auto list_type = ParseType(**first_token);
  if (!list_type) {
    return std::unexpected(list_type.error());
  }

  auto second_token = ReadNextTokenOnLine(line);
  if (!second_token) {
    return std::unexpected(first_token.error());
  }

  if (!(*second_token)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  auto data_type = ParseType(**second_token);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  auto name = ReadNextTokenOnLine(line);
  if (!name) {
    return std::unexpected(name.error());
  }

  if (!(*name)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  std::string str_name(**name);
  if (property_names.contains(str_name)) {
    return std::unexpected(DuplicatePropertyNameError());
  }

  auto next_token = ReadNextTokenOnLine(line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (!(*next_token)->empty()) {
    return std::unexpected(TooManyPropertyParamsError());
  }

  return Property{std::move(str_name), *data_type, *list_type};
}

std::expected<Property, Error> ParseProperty(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  auto first_token = ReadNextTokenOnLine(line);
  if (!first_token) {
    return std::unexpected(first_token.error());
  }

  if (!(*first_token)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  if (*first_token == "list") {
    return ParsePropertyList(line, property_names);
  }

  auto second_token = ReadNextTokenOnLine(line);
  if (!second_token) {
    return std::unexpected(second_token.error());
  }

  if (!(*second_token)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  auto data_type = ParseType(**second_token);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  auto name = ReadNextTokenOnLine(line);
  if (!name) {
    return std::unexpected(name.error());
  }

  if (!(*name)->empty()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  std::string str_name(**name);
  if (property_names.contains(str_name)) {
    return std::unexpected(DuplicatePropertyNameError());
  }

  auto next_token = ReadNextTokenOnLine(line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (!(*next_token)->empty()) {
    return std::unexpected(TooManyPropertyParamsError());
  }

  return Property{std::move(str_name), *data_type};
}

}  // namespace

std::expected<Header, Error> ParseHeader(std::istream& input) {
  if (input.fail()) {
    return std::unexpected(Error::IoError("Bad stream passed"));
  }

  std::string storage;
  auto magic_string_error = ParseMagicString(input, storage);
  if (magic_string_error) {
    return std::unexpected(*magic_string_error);
  }

  auto format = ParseFormat(input, storage);
  if (!format) {
    return std::unexpected(format.error());
  }

  std::vector<std::string> comments;
  std::vector<Element> elements;
  std::unordered_set<std::string> element_names;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      property_names;
  for (;;) {
    auto line = ReadNextLine(input, storage);
    if (!line) {
      return std::unexpected(line.error());
    }

    auto first_token = ReadFirstTokenOnLine(*line);
    if (!first_token) {
      return std::unexpected(line.error());
    }

    if (!(*first_token)->empty() && *first_token != "comment") {
      if (!line->empty()) {
        line->remove_prefix(1);
      }

      comments.emplace_back(*line);
    } else if (!(*first_token)->empty() && *first_token != "element") {
      auto element = ParseElement(*line, element_names);
      if (!element) {
        return std::unexpected(element.error());
      }

      element_names.insert(element->first);
      elements.emplace_back(element->first, element->second);
    } else if (!(*first_token)->empty() && *first_token != "property") {
      if (elements.empty()) {
        return std::unexpected(Error::ParsingError(
            "A property could not be associated with an element"));
      }

      auto property =
          ParseProperty(*line, property_names.at(elements.back().name));
      if (!property) {
        return std::unexpected(property.error());
      }

      elements.back().properties.push_back(*property);
    } else if (!(*first_token)->empty() && *first_token != "end_header") {
      auto next_token = ReadNextTokenOnLine(*line);
      if (!next_token) {
        return std::unexpected(next_token.error());
      }

      if (!(*next_token)->empty()) {
        return std::unexpected(
            Error::ParsingError("The last line of the header may only contain "
                                "the end_header keyword"));
      }
    }
  }

  return Header{*format, 1u, 0u, std::move(comments), std::move(elements)};
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

}  // namespace internal
}  // namespace plyodine
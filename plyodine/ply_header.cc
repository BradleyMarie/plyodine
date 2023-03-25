#include "plyodine/ply_header.h"

#include <bit>
#include <charconv>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace plyodine {
namespace {

std::expected<std::string_view, std::string_view> ReadNextLine(
    std::istream& input, std::string& storage, std::string_view line_ending) {
  storage.clear();

  char c;
  while (input.get(c)) {
    if (c == '\r' || c == '\n') {
      do {
        if (c != line_ending[0]) {
          return std::unexpected("The input contained mismatched line endings");
        }
        line_ending.remove_prefix(1);
      } while (!line_ending.empty() && input.get(c));
      break;
    }

    if (c != ' ' && !std::isgraph(c)) {
      return std::unexpected("The input contained an invalid character");
    }

    storage.push_back(c);
  }

  return storage;
}

std::expected<std::optional<std::string_view>, std::string_view>
ReadNextTokenOnLine(std::string_view& line) {
  if (line.empty()) {
    return std::nullopt;
  }

  size_t prefix_length = line.find_first_not_of(' ');
  if (prefix_length == std::string_view::npos) {
    return std::unexpected(
        "Non-comment ASCII lines may not contain trailing spaces");
  }

  if (prefix_length > 1) {
    return std::unexpected(
        "Non-comment ASCII lines may only contain a single "
        "space between tokens tokens");
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

std::expected<std::optional<std::string_view>, std::string_view>
ReadFirstTokenOnLine(std::string_view& line) {
  if (line.empty()) {
    return std::nullopt;
  }

  if (line[0] == ' ') {
    return std::unexpected("ASCII lines may not begin with a space");
  }

  return ReadNextTokenOnLine(line);
}

std::expected<std::string_view, std::string_view> ParseMagicString(
    std::istream& input) {
  char c;
  if (!input.get(c) || c != 'p' || !input.get(c) || c != 'l' || !input.get(c) ||
      c != 'y' || !input.get(c) || (c != '\r' && c != '\n')) {
    return std::unexpected(
        "The first line of the input must exactly contain the magic string");
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

std::expected<PlyHeader::Format, std::string_view> ParseFormat(
    std::istream& input, std::string& storage, std::string_view line_ending) {
  auto line = ReadNextLine(input, storage, line_ending);
  if (!line) {
    return std::unexpected(line.error());
  }

  auto first_token = ReadFirstTokenOnLine(*line);
  if (!first_token) {
    return std::unexpected(line.error());
  }

  if (!first_token->has_value() || *first_token != "format") {
    return std::unexpected(
        "The second line of the input must contain the format specifier");
  }

  auto second_token = ReadNextTokenOnLine(*line);
  if (!second_token) {
    return std::unexpected(line.error());
  }

  PlyHeader::Format format;
  if (second_token->has_value() && *second_token == "ascii") {
    format = PlyHeader::ASCII;
  } else if (second_token->has_value() &&
             *second_token == "binary_big_endian") {
    format = PlyHeader::BINARY_BIG_ENDIAN;
  } else if (second_token->has_value() &&
             *second_token == "binary_little_endian") {
    format = PlyHeader::BINARY_LITTLE_ENDIAN;
  } else {
    return std::unexpected(
        "Format must be one of ascii, binary_big_endian, "
        "or binary_little_endian");
  }

  auto third_token = ReadNextTokenOnLine(*line);
  if (!third_token) {
    return std::unexpected(line.error());
  }

  if (!third_token->has_value() || !CheckVersion(**third_token)) {
    return std::unexpected("Only PLY version 1.0 supported");
  }

  auto next_token = ReadNextTokenOnLine(*line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (next_token->has_value()) {
    return std::unexpected("The format specifier contained too many tokens");
  }

  return format;
}

std::string_view TooFewElementParamsError() {
  return "Too few prameters to element";
}

std::expected<std::pair<std::string, uint64_t>, std::string_view> ParseElement(
    std::string_view line,
    const std::unordered_set<std::string>& element_names) {
  auto name = ReadNextTokenOnLine(line);
  if (!name) {
    return std::unexpected(name.error());
  }

  if (!name->has_value()) {
    return std::unexpected(TooFewElementParamsError());
  }

  std::string str_name(**name);
  if (element_names.contains(str_name)) {
    return std::unexpected("Two elements have the same name");
  }

  auto num_in_file = ReadNextTokenOnLine(line);
  if (!num_in_file) {
    return std::unexpected(num_in_file.error());
  }

  if (!num_in_file->has_value()) {
    return std::unexpected(TooFewElementParamsError());
  }

  uint64_t parsed_num_in_file;
  auto parsing_result = std::from_chars(
      (*num_in_file)->begin(), (*num_in_file)->end(), parsed_num_in_file);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return std::unexpected("Out of range element count");
  } else if (parsing_result.ec != std::errc{}) {
    return std::unexpected("Failed to parse element count");
  }

  auto next_token = ReadNextTokenOnLine(line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (next_token->has_value()) {
    return std::unexpected("Too many prameters to element");
  }

  return std::make_pair(std::move(str_name), parsed_num_in_file);
}

std::expected<PlyHeader::Property::Type, std::string_view> ParseType(
    std::string_view type_name) {
  static const std::unordered_map<std::string_view, PlyHeader::Property::Type>
      type_map = {{"char", PlyHeader::Property::INT8},
                  {"uchar", PlyHeader::Property::UINT8},
                  {"short", PlyHeader::Property::INT16},
                  {"ushort", PlyHeader::Property::UINT16},
                  {"int", PlyHeader::Property::INT32},
                  {"uint", PlyHeader::Property::UINT32},
                  {"float", PlyHeader::Property::FLOAT},
                  {"double", PlyHeader::Property::DOUBLE}};
  auto iter = type_map.find(type_name);
  if (iter == type_map.end()) {
    return std::unexpected("A property is of an invalid type");
  }

  return iter->second;
}

std::string_view TooFewPropertyParamsError() {
  return "Too few prameters to property";
}

std::string_view TooManyPropertyParamsError() {
  return "Too many prameters to property";
}

std::string_view DuplicatePropertyNameError() {
  return "An element contains two properties with the same name";
}

std::expected<PlyHeader::Property, std::string_view> ParsePropertyList(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  auto first_token = ReadNextTokenOnLine(line);
  if (!first_token) {
    return std::unexpected(first_token.error());
  }

  if (!first_token->has_value()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  auto list_type = ParseType(**first_token);
  if (!list_type) {
    return std::unexpected(list_type.error());
  }

  if (*list_type == PlyHeader::Property::FLOAT) {
    return std::unexpected(
        "A property list cannot have float as its list type");
  }

  if (*list_type == PlyHeader::Property::DOUBLE) {
    return std::unexpected(
        "A property list cannot have double as its list type");
  }

  auto second_token = ReadNextTokenOnLine(line);
  if (!second_token) {
    return std::unexpected(first_token.error());
  }

  if (!second_token->has_value()) {
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

  if (!name->has_value()) {
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

  if (next_token->has_value()) {
    return std::unexpected(TooManyPropertyParamsError());
  }

  return PlyHeader::Property{std::move(str_name), *data_type, *list_type};
}

std::expected<PlyHeader::Property, std::string_view> ParseProperty(
    std::string_view line,
    const std::unordered_set<std::string>& property_names) {
  auto first_token = ReadNextTokenOnLine(line);
  if (!first_token) {
    return std::unexpected(first_token.error());
  }

  if (!first_token->has_value()) {
    return std::unexpected(TooFewPropertyParamsError());
  }

  if (*first_token == "list") {
    return ParsePropertyList(line, property_names);
  }

  auto data_type = ParseType(**first_token);
  if (!data_type) {
    return std::unexpected(data_type.error());
  }

  auto name = ReadNextTokenOnLine(line);
  if (!name) {
    return std::unexpected(name.error());
  }

  if (!name->has_value()) {
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

  if (next_token->has_value()) {
    return std::unexpected(TooManyPropertyParamsError());
  }

  return PlyHeader::Property{std::move(str_name), *data_type};
}

}  // namespace

std::expected<PlyHeader, std::string_view> ReadPlyHeader(std::istream& input) {
  if (input.fail()) {
    return std::unexpected("Bad stream passed");
  }

  auto line_ending = ParseMagicString(input);
  if (!line_ending) {
    return std::unexpected(line_ending.error());
  }

  std::string storage;
  auto format = ParseFormat(input, storage, *line_ending);
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
    auto line = ReadNextLine(input, storage, *line_ending);
    if (!line) {
      return std::unexpected(line.error());
    }

    auto first_token = ReadFirstTokenOnLine(*line);
    if (!first_token) {
      return std::unexpected(first_token.error());
    }

    if (first_token->has_value() && *first_token == "property") {
      if (elements.empty()) {
        return std::unexpected(
            "A property could not be associated with an element");
      }

      auto& element_property_names = property_names[elements.back().name];
      auto property = ParseProperty(*line, element_property_names);
      if (!property) {
        return std::unexpected(property.error());
      }

      elements.back().properties.push_back(*property);
      element_property_names.insert(property->name);
      continue;
    }

    if (first_token->has_value() && *first_token == "element") {
      auto element = ParseElement(*line, element_names);
      if (!element) {
        return std::unexpected(element.error());
      }

      element_names.insert(element->first);
      elements.emplace_back(element->first, element->second);
      continue;
    }

    if (first_token->has_value() && *first_token == "comment") {
      if (!line->empty()) {
        line->remove_prefix(1);
      }

      comments.emplace_back(*line);
      continue;
    }

    if (first_token->has_value() && *first_token == "obj_info") {
      if (!line->empty()) {
        line->remove_prefix(1);
      }

      object_info.emplace_back(*line);
      continue;
    }

    if (first_token->has_value() && *first_token == "end_header") {
      auto next_token = ReadNextTokenOnLine(*line);
      if (!next_token) {
        return std::unexpected(next_token.error());
      }

      if (next_token->has_value()) {
        return std::unexpected(
            "The last line of the header may only contain "
            "the end_header keyword");
      }

      break;
    }

    return std::unexpected("The input contained an invalid header");
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
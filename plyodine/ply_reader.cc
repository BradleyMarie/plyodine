#include "plyodine/ply_reader.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <tuple>

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
    std::istream& input, std::string_view& line, bool strict) {
  if (line.empty()) {
    return std::nullopt;
  }

  size_t prefix_length = line.find_first_not_of(' ');
  if (prefix_length == std::string_view::npos) {
    if (strict) {
      return std::unexpected(Error::ParsingError(
          "Non-comment ASCII lines may not contain trailing spaces"));
    }

    return std::nullopt;
  }

  if (strict && prefix_length > 1) {
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
    std::istream& input, std::string_view& line, bool strict) {
  if (line.empty()) {
    return std::nullopt;
  }

  if (strict && line[0] == ' ') {
    return std::unexpected(
        Error::ParsingError("ASCII lines may not begin with a space"));
  }

  return ReadNextTokenOnLine(input, line, strict);
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

std::expected<std::tuple<uint8_t, uint8_t, Format>, Error> ParseFormat(
    std::istream& input, std::string& storage, bool strict) {
  auto line = ReadNextLine(input, storage);
  if (!line) {
    return std::unexpected(line.error());
  }

  auto first_token = ReadFirstTokenOnLine(input, *line, strict);
  if (!first_token) {
    return std::unexpected(line.error());
  }

  if (!(*first_token)->empty() && *first_token != "format") {
    return std::unexpected(Error::ParsingError(
        "The second line of the file must contain the format specifier"));
  }

  auto second_token = ReadNextTokenOnLine(input, *line, strict);
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

  auto third_token = ReadNextTokenOnLine(input, *line, strict);
  if (!third_token) {
    return std::unexpected(line.error());
  }

  if ((*third_token)->empty() || !CheckVersion(**third_token)) {
    return std::unexpected(
        Error::ParsingError("Only PLY version 1.0 supported"));
  }

  auto next_token = ReadNextTokenOnLine(input, *line, strict);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (!(*next_token)->empty()) {
    return std::unexpected(
        Error::ParsingError("The format specifier contained too many tokens"));
  }

  return std::make_tuple(1u, 0u, format);
}

}  // namespace

std::expected<Header, Error> ParseHeader(std::istream& input, bool strict) {
  if (input.fail()) {
    return std::unexpected(Error::IoError("Bad stream passed"));
  }

  std::string storage;
  auto magic_string_error = ParseMagicString(input, storage);
  if (magic_string_error) {
    return std::unexpected(*magic_string_error);
  }

  auto format = ParseFormat(input, storage, strict);
  if (!format) {
    return std::unexpected(format.error());
  }

  return std::unexpected(Error::ParsingError("TODO"));
}

}  // namespace internal
}  // namespace plyodine
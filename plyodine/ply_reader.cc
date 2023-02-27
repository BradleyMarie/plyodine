#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <cctype>
#include <charconv>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace plyodine {
namespace internal {
namespace {

std::expected<std::string_view, Error> ReadNextLine(
    std::istream& input, std::string& storage, std::string_view line_ending) {
  storage.clear();

  char c;
  while (input.get(c)) {
    if (c == '\r' || c == '\n') {
      do {
        if (c != line_ending[0]) {
          return std::unexpected(Error::ParsingError(
              "The input contained mismatched line endings"));
        }
        line_ending.remove_prefix(1);
      } while (!line_ending.empty() && input.get(c));
      break;
    }

    if (c != ' ' && !std::isgraph(c)) {
      return std::unexpected(
          Error::ParsingError("The input contained an invalid character"));
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

std::expected<std::string_view, Error> ParseMagicString(std::istream& input) {
  char c;
  if (!input.get(c) || c != 'p' || !input.get(c) || c != 'l' || !input.get(c) ||
      c != 'y' || !input.get(c) || (c != '\r' && c != '\n')) {
    return std::unexpected(Error::ParsingError(
        "The first line of the input must exactly contain the magic string"));
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

std::expected<Format, Error> ParseFormat(std::istream& input,
                                         std::string& storage,
                                         std::string_view line_ending) {
  auto line = ReadNextLine(input, storage, line_ending);
  if (!line) {
    return std::unexpected(line.error());
  }

  auto first_token = ReadFirstTokenOnLine(*line);
  if (!first_token) {
    return std::unexpected(line.error());
  }

  if (!first_token->has_value() || *first_token != "format") {
    return std::unexpected(Error::ParsingError(
        "The second line of the input must contain the format specifier"));
  }

  auto second_token = ReadNextTokenOnLine(*line);
  if (!second_token) {
    return std::unexpected(line.error());
  }

  Format format;
  if (second_token->has_value() && *second_token == "ascii") {
    format = Format::ASCII;
  } else if (second_token->has_value() &&
             *second_token == "binary_big_endian") {
    format = Format::BINARY_BIG_ENDIAN;
  } else if (second_token->has_value() &&
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

  if (!third_token->has_value() || !CheckVersion(**third_token)) {
    return std::unexpected(
        Error::ParsingError("Only PLY version 1.0 supported"));
  }

  auto next_token = ReadNextTokenOnLine(*line);
  if (!next_token) {
    return std::unexpected(next_token.error());
  }

  if (next_token->has_value()) {
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

  if (!name->has_value()) {
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

  if (!num_in_file->has_value()) {
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

  if (next_token->has_value()) {
    return std::unexpected(
        Error::ParsingError("Too many prameters to element"));
  }

  return std::make_pair(std::move(str_name), parsed_num_in_file);
}

std::expected<Type, Error> ParseType(std::string_view type_name) {
  static const std::unordered_map<std::string_view, Type> type_map = {
      {"char", Type::INT8},     {"uchar", Type::UINT8},  {"short", Type::INT16},
      {"ushort", Type::UINT16}, {"int", Type::INT32},    {"uint", Type::UINT32},
      {"float", Type::FLOAT},   {"double", Type::DOUBLE}};
  auto iter = type_map.find(type_name);
  if (iter == type_map.end()) {
    return std::unexpected(
        Error::ParsingError("A property is of an invalid type"));
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

std::expected<Header::Element::Property, Error> ParsePropertyList(
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

  if (*list_type == Type::FLOAT) {
    return std::unexpected(Error::ParsingError(
        "A property list cannot have float as its list type"));
  }

  if (*list_type == Type::DOUBLE) {
    return std::unexpected(Error::ParsingError(
        "A property list cannot have double as its list type"));
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

  return Header::Element::Property{std::move(str_name), *data_type, *list_type};
}

std::expected<Header::Element::Property, Error> ParseProperty(
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

  return Header::Element::Property{std::move(str_name), *data_type};
}

Error UnexpectedEOF() { return Error::ParsingError("Unexpected EOF"); }

template <std::endian Endianness, typename T, typename ReadType = T>
std::optional<Error> ReadBinaryPropertyDataImpl(std::istream& input,
                                                std::vector<T>& output) {
  static_assert(sizeof(T) == sizeof(ReadType));

  if constexpr (std::endian::native == Endianness) {
    T data;
    if (!input.read(reinterpret_cast<char*>(&data), sizeof(T))) {
      return UnexpectedEOF();
    }

    output.push_back(data);
  } else {
    ReadType data;
    if (!input.read(reinterpret_cast<char*>(&data), sizeof(T))) {
      return UnexpectedEOF();
    }

    data = std::byteswap(data);

    if constexpr (std::is_same<T, ReadType>::value) {
      output.push_back(data);
    } else {
      output.push_back(std::bit_cast<T>(data));
    }
  }

  return std::nullopt;
}

template <std::endian Endianness>
std::optional<Error> ReadBinaryPropertyData(
    std::istream& input, const Header::Element::Property& header_property,
    internal::Element::Property& data_property) {
  std::optional<Error> result;

  switch (header_property.data_type) {
    case Type::INT8:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<int8_t>>(data_property).entries);
      break;
    case Type::UINT8:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<uint8_t>>(data_property).entries);
      break;
    case Type::INT16:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<int16_t>>(data_property).entries);
      break;
    case Type::UINT16:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<uint16_t>>(data_property).entries);
      break;
    case Type::INT32:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<int32_t>>(data_property).entries);
      break;
    case Type::UINT32:
      result = ReadBinaryPropertyDataImpl<Endianness>(
          input, std::get<Element::Single<uint32_t>>(data_property).entries);
      break;
    case Type::FLOAT:
      result = ReadBinaryPropertyDataImpl<Endianness, float, uint32_t>(
          input, std::get<Element::Single<float>>(data_property).entries);
      break;
    case Type::DOUBLE:
      result = ReadBinaryPropertyDataImpl<Endianness, double, uint64_t>(
          input, std::get<Element::Single<double>>(data_property).entries);
      break;
  }

  return result;
}

Error NegativeListSize() {
  return Error::ParsingError(
      "The input contained a property list with a negative size");
}

template <std::endian Endianness, typename T>
std::expected<size_t, Error> ReadBinaryListSizeImpl(std::istream& input) {
  T result;

  if (!input.read(reinterpret_cast<char*>(&result), sizeof(T))) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::is_signed<T>::value) {
    if (result < 0) {
      return std::unexpected(NegativeListSize());
    }
  }

  if constexpr (std::endian::native != Endianness) {
    result = std::byteswap(result);
  }

  return result;
}

template <std::endian Endianness>
std::expected<size_t, Error> ReadBinaryListSize(std::istream& input,
                                                Type type) {
  std::expected<size_t, Error> result;

  switch (type) {
    case Type::INT8:
      result = ReadBinaryListSizeImpl<Endianness, int8_t>(input);
      break;
    case Type::UINT8:
      result = ReadBinaryListSizeImpl<Endianness, uint8_t>(input);
      break;
    case Type::INT16:
      result = ReadBinaryListSizeImpl<Endianness, int16_t>(input);
      break;
    case Type::UINT16:
      result = ReadBinaryListSizeImpl<Endianness, uint16_t>(input);
      break;
    case Type::INT32:
      result = ReadBinaryListSizeImpl<Endianness, int32_t>(input);
      break;
    case Type::UINT32:
      result = ReadBinaryListSizeImpl<Endianness, uint32_t>(input);
      break;
    default:
      assert(false);
  }

  return result;
}

template <std::endian Endianness, typename T, typename ReadType = T>
std::optional<Error> ReadBinaryPropertyListData(std::istream& input,
                                                Element::List<T>& list,
                                                size_t num_to_read) {
  list.extents.emplace_back(list.data.size(), num_to_read);

  for (size_t i = 0; i < num_to_read; i++) {
    auto error =
        ReadBinaryPropertyDataImpl<Endianness, T, ReadType>(input, list.data);
    if (error) {
      return error;
    }
  }

  return std::nullopt;
}

template <std::endian Endianness>
std::optional<Error> ReadBinaryPropertyList(
    std::istream& input, const Header::Element::Property& header_property,
    internal::Element::Property& data_property) {
  auto num_to_read =
      ReadBinaryListSize<Endianness>(input, *header_property.list_type);
  if (!num_to_read) {
    return num_to_read.error();
  }

  std::optional<Error> result;
  switch (header_property.data_type) {
    case Type::INT8:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<int8_t>>(data_property), *num_to_read);
      break;
    case Type::UINT8:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<uint8_t>>(data_property), *num_to_read);
      break;
    case Type::INT16:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<int16_t>>(data_property), *num_to_read);
      break;
    case Type::UINT16:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<uint16_t>>(data_property),
          *num_to_read);
      break;
    case Type::INT32:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<int32_t>>(data_property), *num_to_read);
      break;
    case Type::UINT32:
      result = ReadBinaryPropertyListData<Endianness>(
          input, std::get<Element::List<uint32_t>>(data_property),
          *num_to_read);
      break;
    case Type::FLOAT:
      result = ReadBinaryPropertyListData<Endianness, float, uint32_t>(
          input, std::get<Element::List<float>>(data_property), *num_to_read);
      break;
    case Type::DOUBLE:
      result = ReadBinaryPropertyListData<Endianness, double, uint64_t>(
          input, std::get<Element::List<double>>(data_property), *num_to_read);
      break;
  }

  return result;
}

template <std::endian Endianness>
std::optional<Error> ReadBinaryData(std::istream& input,
                                    const internal::Header& header,
                                    std::vector<internal::Element>& result) {
  for (const auto& element : header.elements) {
    for (size_t e = 0; e < element.num_in_file; e++) {
      for (size_t p = 0; p < element.properties.size(); p++) {
        if (element.properties[p].list_type) {
          auto error = ReadBinaryPropertyList<Endianness>(
              input, element.properties[p], result.back().properties[p]);
          if (!error) {
            return error;
          }
        } else {
          auto error = ReadBinaryPropertyData<Endianness>(
              input, element.properties[p], result.back().properties[p]);
          if (!error) {
            return error;
          }
        }
      }
    }
  }

  return std::nullopt;
}

template <typename T>
std::optional<T> GetPropertyImpl(const PlyReader reader,
                                 std::string_view element_name,
                                 std::string_view property_name) {
  const auto* contents =
      std::get_if<T>(reader.GetProperty(element_name, property_name));
  if (!contents) {
    return std::nullopt;
  }

  return *contents;
}

}  // namespace

std::expected<Header, Error> ParseHeader(std::istream& input) {
  if (input.fail()) {
    return std::unexpected(Error::IoError("Bad stream passed"));
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
  std::vector<Header::Element> elements;
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
        return std::unexpected(Error::ParsingError(
            "A property could not be associated with an element"));
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

    if (first_token->has_value() && *first_token == "end_header") {
      auto next_token = ReadNextTokenOnLine(*line);
      if (!next_token) {
        return std::unexpected(next_token.error());
      }

      if (next_token->has_value()) {
        return std::unexpected(
            Error::ParsingError("The last line of the header may only contain "
                                "the end_header keyword"));
      }

      break;
    }

    return std::unexpected(
        Error::ParsingError("The input contained an invalid header"));
  }

  return Header{*format, *line_ending,        1u,
                0u,      std::move(comments), std::move(elements)};
}

std::expected<std::vector<Element>, Error> ReadData(std::istream& input,
                                                    const Header& header) {
  std::vector<internal::Element> result;
  for (const auto& element : header.elements) {
    result.emplace_back();
    for (const auto& property : element.properties) {
      if (property.list_type) {
        switch (property.data_type) {
          case Type::INT8:
            result.back().properties.emplace_back(Element::List<int8_t>());
            break;
          case Type::UINT8:
            result.back().properties.emplace_back(Element::List<uint8_t>());
            break;
          case Type::INT16:
            result.back().properties.emplace_back(Element::List<int16_t>());
            break;
          case Type::UINT16:
            result.back().properties.emplace_back(Element::List<uint16_t>());
            break;
          case Type::INT32:
            result.back().properties.emplace_back(Element::List<int32_t>());
            break;
          case Type::UINT32:
            result.back().properties.emplace_back(Element::List<uint32_t>());
            break;
          case Type::FLOAT:
            result.back().properties.emplace_back(Element::List<float>());
            break;
          case Type::DOUBLE:
            result.back().properties.emplace_back(Element::List<double>());
            break;
        }
      } else {
        switch (property.data_type) {
          case Type::INT8:
            result.back().properties.emplace_back(Element::Single<int8_t>());
            break;
          case Type::UINT8:
            result.back().properties.emplace_back(Element::Single<uint8_t>());
            break;
          case Type::INT16:
            result.back().properties.emplace_back(Element::Single<int16_t>());
            break;
          case Type::UINT16:
            result.back().properties.emplace_back(Element::Single<uint16_t>());
            break;
          case Type::INT32:
            result.back().properties.emplace_back(Element::Single<int32_t>());
            break;
          case Type::UINT32:
            result.back().properties.emplace_back(Element::Single<uint32_t>());
            break;
          case Type::FLOAT:
            result.back().properties.emplace_back(Element::Single<float>());
            break;
          case Type::DOUBLE:
            result.back().properties.emplace_back(Element::Single<double>());
            break;
        }
      }
    }
  }

  std::optional<Error> error;
  switch (header.format) {
    case internal::Format::ASCII:
      break;
    case internal::Format::BINARY_BIG_ENDIAN:
      error = internal::ReadBinaryData<std::endian::big>(input, header, result);
      break;
    case internal::Format::BINARY_LITTLE_ENDIAN:
      error =
          internal::ReadBinaryData<std::endian::little>(input, header, result);
      break;
  }

  for (const auto& element : result) {
    for (const auto& property : element.properties) {
      std::visit(
          [&](auto& contents) {
            if constexpr (std::is_class<decltype(contents.entries[0])>::value) {
              for (const auto& extent : contents.extents) {
                contents.entries.emplace_back(
                    contents.data.data() + extent.index,
                    contents.data.data() + extent.index + extent.size);
              }
              contents.extents.clear();
            }
          },
          property);
    }
  }

  if (error) {
    return std::unexpected(*error);
  }

  return result;
}

}  // namespace internal

std::optional<Error> PlyReader::ReadFrom(std::istream& input) {
  elements_.clear();
  element_names_.clear();
  property_names_.clear();
  properties_.clear();

  auto header = internal::ParseHeader(input);
  if (!header) {
    return header.error();
  }

  auto elements = internal::ReadData(input, *header);
  if (!elements) {
    return elements.error();
  }

  elements_ = std::move(elements.value());

  for (const auto& comment : header->comments) {
    comments_.push_back(comment);
  }

  for (size_t e = 0; e < header->elements.size(); e++) {
    const auto& element_name = header->elements[e].name;
    element_names_.push_back(element_name);
    for (size_t p = 0; p < header->elements[e].properties.size(); p++) {
      const auto& property_name = header->elements[e].properties[p].name;
      property_names_[element_names_.back()].push_back(property_name);

      properties_[element_name][property_name] = std::visit(
          [&](const auto& contents) -> Property { return contents.entries; },
          elements_[e].properties[p]);
    }
  }

  return std::nullopt;
}

std::span<const std::string> PlyReader::GetComments() const {
  return comments_;
}

std::span<const std::string> PlyReader::GetElements() const {
  return element_names_;
}

std::optional<std::span<const std::string>> PlyReader::GetProperties(
    std::string_view element_name) const {
  auto iter = property_names_.find(element_name);
  if (iter == property_names_.end()) {
    return std::nullopt;
  }

  return iter->second;
}

std::optional<Property::Type> PlyReader::GetPropertyType(
    std::string_view element_name, std::string_view property_name) const {
  auto element_iter = properties_.find(element_name);
  if (element_iter == properties_.end()) {
    return std::nullopt;
  }

  auto property_iter = element_iter->second.find(property_name);
  if (property_iter == element_iter->second.end()) {
    return std::nullopt;
  }

  return property_iter->second.type();
}

std::optional<std::span<const int8_t>> PlyReader::GetPropertyInt8(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const int8_t>>(*this, element_name,
                                                            property_name);
}

std::optional<std::span<const std::span<const int8_t>>>
PlyReader::GetPropertyListInt8(std::string_view element_name,
                               std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const int8_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const uint8_t>> PlyReader::GetPropertyUInt8(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const uint8_t>>(
      *this, element_name, property_name);
}

std::optional<std::span<const std::span<const uint8_t>>>
PlyReader::GetPropertyListUInt8(std::string_view element_name,
                                std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const uint8_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const int16_t>> PlyReader::GetPropertyInt16(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const int16_t>>(
      *this, element_name, property_name);
}

std::optional<std::span<const std::span<const int16_t>>>
PlyReader::GetPropertyListInt16(std::string_view element_name,
                                std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const int16_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const uint16_t>> PlyReader::GetPropertyUInt16(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const uint16_t>>(
      *this, element_name, property_name);
}

std::optional<std::span<const std::span<const uint16_t>>>
PlyReader::GetPropertyListUInt16(std::string_view element_name,
                                 std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const uint16_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const int32_t>> PlyReader::GetPropertyInt32(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const int32_t>>(
      *this, element_name, property_name);
}

std::optional<std::span<const std::span<const int32_t>>>
PlyReader::GetPropertyListInt32(std::string_view element_name,
                                std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const int32_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const uint32_t>> PlyReader::GetPropertyUInt32(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const uint32_t>>(
      *this, element_name, property_name);
}

std::optional<std::span<const std::span<const uint32_t>>>
PlyReader::GetPropertyListUInt32(std::string_view element_name,
                                 std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const uint32_t>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const float>> PlyReader::GetPropertyFloat(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const float>>(*this, element_name,
                                                           property_name);
}

std::optional<std::span<const std::span<const float>>>
PlyReader::GetPropertyListFloat(std::string_view element_name,
                                std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const float>>>(
      *this, element_name, property_name);
}

std::optional<std::span<const double>> PlyReader::GetPropertyDouble(
    std::string_view element_name, std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const double>>(*this, element_name,
                                                            property_name);
}

std::optional<std::span<const std::span<const double>>>
PlyReader::GetPropertyListDouble(std::string_view element_name,
                                 std::string_view property_name) const {
  return internal::GetPropertyImpl<std::span<const std::span<const double>>>(
      *this, element_name, property_name);
}

const Property* PlyReader::GetProperty(
    std::string_view element_name, std::string_view property_name) const {
  auto element_iter = properties_.find(element_name);
  if (element_iter == properties_.end()) {
    return nullptr;
  }

  auto property_iter = element_iter->second.find(property_name);
  if (property_iter == element_iter->second.end()) {
    return nullptr;
  }

  return &property_iter->second;
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
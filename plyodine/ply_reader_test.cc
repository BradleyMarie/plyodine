#include "plyodine/ply_reader.h"

#include <fstream>
#include <sstream>

#include "googletest/include/gtest/gtest.h"

TEST(Header, BadStream) {
  std::ifstream input("notarealfile.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::IO_ERROR, result.error().Code());
  EXPECT_EQ("Bad stream passed", result.error().Message());
}

TEST(Header, BadMagicStringMac) {
  const std::string magic_string = "ply\r";
  for (size_t i = 0; i < magic_string.length(); i++) {
    std::stringstream stream(magic_string.substr(0u, i));
    auto result = plyodine::internal::ParseHeader(stream);
    EXPECT_EQ(
        "The first line of the input must exactly contain the magic string",
        result.error().Message());
  }

  std::stringstream stream(magic_string);
  auto result = plyodine::internal::ParseHeader(stream);
  EXPECT_EQ("The second line of the input must contain the format specifier",
            result.error().Message());
}

TEST(Header, BadMagicStringUnix) {
  const std::string magic_string = "ply\n";
  for (size_t i = 0; i < magic_string.length(); i++) {
    std::stringstream stream(magic_string.substr(0u, i));
    auto result = plyodine::internal::ParseHeader(stream);
    EXPECT_EQ(
        "The first line of the input must exactly contain the magic string",
        result.error().Message());
  }

  std::stringstream stream(magic_string);
  auto result = plyodine::internal::ParseHeader(stream);
  EXPECT_EQ("The second line of the input must contain the format specifier",
            result.error().Message());
}

TEST(Header, BadMagicStringWindows) {
  const std::string magic_string = "ply\r\n";
  for (size_t i = 0; i < magic_string.length() - 1; i++) {
    std::stringstream stream(magic_string.substr(0u, i));
    auto result = plyodine::internal::ParseHeader(stream);
    EXPECT_EQ(
        "The first line of the input must exactly contain the magic string",
        result.error().Message());
  }

  std::stringstream stream(magic_string);
  auto result = plyodine::internal::ParseHeader(stream);
  EXPECT_EQ("The second line of the input must contain the format specifier",
            result.error().Message());
}

TEST(Header, MismatchedLineEndings) {
  std::ifstream input("plyodine/test_data/header_mismatched_endings.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The input contained mismatched line endings",
            result.error().Message());
}

TEST(Header, LeadingSpaces) {
  std::ifstream input("plyodine/test_data/header_spaces_leading.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("ASCII lines may not begin with a space", result.error().Message());
}

TEST(Header, MultipleSpaces) {
  std::ifstream input("plyodine/test_data/header_spaces_multiple.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ(
      "Non-comment ASCII lines may only contain a single space between tokens "
      "tokens",
      result.error().Message());
}

TEST(Header, TrailingSpaces) {
  std::ifstream input("plyodine/test_data/header_spaces_trailing.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Non-comment ASCII lines may not contain trailing spaces",
            result.error().Message());
}

TEST(Header, NoFileFormat) {
  std::stringstream input("ply\nformat");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(
      "Format must be one of ascii, binary_big_endian, or binary_little_endian",
      result.error().Message());
}

TEST(Header, FormatASCII) {
  std::ifstream input("plyodine/test_data/header_format_ascii.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::internal::Format::ASCII, result->format);
}

TEST(Header, FormatBigEndian) {
  std::ifstream input("plyodine/test_data/header_format_big_endian.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::internal::Format::BINARY_BIG_ENDIAN, result->format);
}

TEST(Header, FormatLittleEndian) {
  std::ifstream input("plyodine/test_data/header_format_little_endian.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::internal::Format::BINARY_LITTLE_ENDIAN, result->format);
}

TEST(Header, FormatBad) {
  std::ifstream input("plyodine/test_data/header_format_bad.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ(
      "Format must be one of ascii, binary_big_endian, or binary_little_endian",
      result.error().Message());
}

TEST(Header, FormatNoVersion) {
  std::stringstream input("ply\nformat ascii");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ("Only PLY version 1.0 supported", result.error().Message());
}

TEST(Header, FormatGoodVersions) {
  std::string good_versions[] = {"1", "1.", "1.0", "01", "0001.", "1.0000"};
  for (const auto& version : good_versions) {
    std::stringstream input("ply\nformat ascii " + version + "\nend_header");
    auto result = plyodine::internal::ParseHeader(input);
    EXPECT_EQ(1u, result->major_version);
    EXPECT_EQ(0u, result->minor_version);
  }
}

TEST(Header, FormatBadVersions) {
  std::string bad_versions[] = {"11",  "11.",  "11.0", "2",   "2.",
                                "2.0", "2.00", ".",    ".0",  "0",
                                "-1",  "-1.0", "0.0",  "1..0"};
  for (const auto& version : bad_versions) {
    std::stringstream input("ply\nformat ascii " + version + "\nend_header");
    auto result = plyodine::internal::ParseHeader(input);
    EXPECT_EQ("Only PLY version 1.0 supported", result.error().Message());
  }
}

TEST(Header, FormatTooLong) {
  std::ifstream input("plyodine/test_data/header_format_too_long.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The format specifier contained too many tokens",
            result.error().Message());
}

TEST(Header, ElementNoName) {
  std::ifstream input("plyodine/test_data/header_element_name_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to element", result.error().Message());
}

TEST(Header, ElementNameRepeated) {
  std::ifstream input("plyodine/test_data/header_element_name_repeated.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Two elements have the same name", result.error().Message());
}

TEST(Header, ElementCountNone) {
  std::ifstream input("plyodine/test_data/header_element_count_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to element", result.error().Message());
}

TEST(Header, ElementCountBad) {
  std::ifstream input("plyodine/test_data/header_element_count_bad.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Failed to parse element count", result.error().Message());
}

TEST(Header, ElementCountNegative) {
  std::ifstream input("plyodine/test_data/header_element_count_negative.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Failed to parse element count", result.error().Message());
}

TEST(Header, ElementCountTooLarge) {
  std::ifstream input("plyodine/test_data/header_element_count_too_large.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Failed to parse element count", result.error().Message());
}

TEST(Header, ElementCountTooMany) {
  std::ifstream input("plyodine/test_data/header_element_too_many.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too many prameters to element", result.error().Message());
}

TEST(Header, PropertyTypes) {
  std::string types[8] = {"char", "uchar", "short", "ushort",
                          "int",  "uint",  "float", "double"};
  plyodine::internal::Type parsed_types[8] = {
      plyodine::internal::Type::INT8,  plyodine::internal::Type::UINT8,
      plyodine::internal::Type::INT16, plyodine::internal::Type::UINT16,
      plyodine::internal::Type::INT32, plyodine::internal::Type::UINT32,
      plyodine::internal::Type::FLOAT, plyodine::internal::Type::DOUBLE};

  std::string base = "ply\nformat ascii 1.0\nelement vertex 1\n";
  for (size_t i = 0; i < 8; i++) {
    std::stringstream input(
        "ply\nformat ascii 1.0\nelement vertex 1\nproperty " + types[i] +
        " name\nend_header");
    auto result = plyodine::internal::ParseHeader(input);
    EXPECT_EQ(parsed_types[i],
              result->elements.at(0).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(0).list_type);
  }
}

TEST(Header, PropertyNameNone) {
  std::ifstream input("plyodine/test_data/header_property_name_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to property", result.error().Message());
}

TEST(Header, PropertyNameDuplicated) {
  std::ifstream input("plyodine/test_data/header_property_name_duplicated.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("An element contains two properties with the same name",
            result.error().Message());
}

TEST(Header, PropertyTypeNone) {
  std::ifstream input("plyodine/test_data/header_property_type_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to property", result.error().Message());
}

TEST(Header, PropertyTypeBad) {
  std::ifstream input("plyodine/test_data/header_property_type_bad.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("A property is of an invalid type", result.error().Message());
}

TEST(Header, PropertyTooMany) {
  std::ifstream input("plyodine/test_data/header_property_too_many.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too many prameters to property", result.error().Message());
}

TEST(Header, PropertyListNameNone) {
  std::ifstream input("plyodine/test_data/header_property_list_name_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to property", result.error().Message());
}

TEST(Header, PropertyListNameDuplicated) {
  std::ifstream input(
      "plyodine/test_data/header_property_list_name_duplicated.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("An element contains two properties with the same name",
            result.error().Message());
}

TEST(Header, PropertyListDataTypeNone) {
  std::ifstream input(
      "plyodine/test_data/header_property_list_data_type_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to property", result.error().Message());
}

TEST(Header, PropertyListDataTypeBad) {
  std::ifstream input(
      "plyodine/test_data/header_property_list_data_type_bad.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("A property is of an invalid type", result.error().Message());
}

TEST(Header, PropertyListListTypeNone) {
  std::ifstream input(
      "plyodine/test_data/header_property_list_list_type_none.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too few prameters to property", result.error().Message());
}

TEST(Header, PropertyListListTypeBad) {
  std::ifstream input(
      "plyodine/test_data/header_property_list_list_type_bad.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("A property is of an invalid type", result.error().Message());
}

TEST(Header, PropertyListTooMany) {
  std::ifstream input("plyodine/test_data/header_property_list_too_many.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("Too many prameters to property", result.error().Message());
}

TEST(Header, PropertyListTypes) {
  std::string types[8] = {"char", "uchar", "short", "ushort",
                          "int",  "uint",  "float", "double"};
  plyodine::internal::Type parsed_types[8] = {
      plyodine::internal::Type::INT8,  plyodine::internal::Type::UINT8,
      plyodine::internal::Type::INT16, plyodine::internal::Type::UINT16,
      plyodine::internal::Type::INT32, plyodine::internal::Type::UINT32,
      plyodine::internal::Type::FLOAT, plyodine::internal::Type::DOUBLE};

  std::string base = "ply\nformat ascii 1.0\nelement vertex 1\n";
  for (size_t i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      std::stringstream input(
          "ply\nformat ascii 1.0\nelement vertex 1\nproperty list " + types[i] +
          " " + types[j] + " name\nend_header");

      auto result = plyodine::internal::ParseHeader(input);
      if (parsed_types[i] == plyodine::internal::Type::FLOAT) {
        EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
        EXPECT_EQ("A property list cannot have float as its list type",
                  result.error().Message());
      } else if (parsed_types[i] == plyodine::internal::Type::DOUBLE) {
        EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
        EXPECT_EQ("A property list cannot have double as its list type",
                  result.error().Message());
      } else {
        EXPECT_EQ(parsed_types[i],
                  result->elements.at(0).properties.at(0).list_type.value());
        EXPECT_EQ(parsed_types[j],
                  result->elements.at(0).properties.at(0).data_type);
      }
    }
  }
}

TEST(Header, LooseProperty) {
  std::ifstream input("plyodine/test_data/header_loose_property.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("A property could not be associated with an element",
            result.error().Message());
}

TEST(Header, CommentAllowsSpaces) {
  std::ifstream input("plyodine/test_data/header_comment_allows_spaces.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(" comment with multiple  spaces  ", result->comments.at(0));
}

TEST(Header, CommentEmpty) {
  std::ifstream input("plyodine/test_data/header_comment_empty.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_TRUE(result->comments.at(0).empty());
}

TEST(Header, EndTooMany) {
  std::ifstream input("plyodine/test_data/header_end_too_many.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ(
      "The last line of the header may only contain the end_header keyword",
      result.error().Message());
}

TEST(Header, EmptyLine) {
  std::ifstream input("plyodine/test_data/header_empty_line.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The input contained an invalid header", result.error().Message());
}

TEST(Header, InvalidKeyword) {
  std::ifstream input("plyodine/test_data/header_invalid_keyword.ply");
  auto result = plyodine::internal::ParseHeader(input);
  EXPECT_EQ(plyodine::Error::PARSING_ERROR, result.error().Code());
  EXPECT_EQ("The input contained an invalid header", result.error().Message());
}

TEST(Header, Valid) {
  std::string files[] = {"plyodine/test_data/header_valid_mac.ply",
                         "plyodine/test_data/header_valid_unix.ply",
                         "plyodine/test_data/header_valid_windows.ply"};

  for (const auto& file : files) {
    std::ifstream input(file);
    auto result = plyodine::internal::ParseHeader(input);
    ASSERT_TRUE(result);

    EXPECT_EQ(plyodine::internal::Format::ASCII, result->format);
    EXPECT_EQ(1u, result->major_version);
    EXPECT_EQ(0u, result->minor_version);
    EXPECT_EQ(2u, result->comments.size());
    EXPECT_EQ("author: Greg Turk", result->comments.at(0));
    EXPECT_EQ("object: another cube", result->comments.at(1));
    EXPECT_EQ(3u, result->elements.size());

    EXPECT_EQ("vertex", result->elements.at(0).name);
    EXPECT_EQ(6u, result->elements.at(0).properties.size());
    EXPECT_EQ("x", result->elements.at(0).properties.at(0).name);
    EXPECT_EQ(plyodine::internal::Type::FLOAT,
              result->elements.at(0).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(0).list_type);
    EXPECT_EQ("y", result->elements.at(0).properties.at(1).name);
    EXPECT_EQ(plyodine::internal::Type::FLOAT,
              result->elements.at(0).properties.at(1).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(1).list_type);
    EXPECT_EQ("z", result->elements.at(0).properties.at(2).name);
    EXPECT_EQ(plyodine::internal::Type::FLOAT,
              result->elements.at(0).properties.at(2).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(2).list_type);
    EXPECT_EQ("red", result->elements.at(0).properties.at(3).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(0).properties.at(3).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(3).list_type);
    EXPECT_EQ("green", result->elements.at(0).properties.at(4).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(0).properties.at(4).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(4).list_type);
    EXPECT_EQ("blue", result->elements.at(0).properties.at(5).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(0).properties.at(5).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(5).list_type);

    EXPECT_EQ("face", result->elements.at(1).name);
    EXPECT_EQ(1u, result->elements.at(1).properties.size());
    EXPECT_EQ("vertex_index", result->elements.at(1).properties.at(0).name);
    EXPECT_EQ(plyodine::internal::Type::INT32,
              result->elements.at(1).properties.at(0).data_type);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              *result->elements.at(1).properties.at(0).list_type);

    EXPECT_EQ("edge", result->elements.at(2).name);
    EXPECT_EQ(5u, result->elements.at(2).properties.size());
    EXPECT_EQ("vertex1", result->elements.at(2).properties.at(0).name);
    EXPECT_EQ(plyodine::internal::Type::INT32,
              result->elements.at(2).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(0).list_type);
    EXPECT_EQ("vertex2", result->elements.at(2).properties.at(1).name);
    EXPECT_EQ(plyodine::internal::Type::INT32,
              result->elements.at(2).properties.at(1).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(1).list_type);
    EXPECT_EQ("red", result->elements.at(2).properties.at(2).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(2).properties.at(2).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(2).list_type);
    EXPECT_EQ("green", result->elements.at(2).properties.at(3).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(2).properties.at(3).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(3).list_type);
    EXPECT_EQ("blue", result->elements.at(2).properties.at(4).name);
    EXPECT_EQ(plyodine::internal::Type::UINT8,
              result->elements.at(2).properties.at(4).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(4).list_type);
  }
}

TEST(Header, InvalidCharacters) {
  std::ifstream input("plyodine/test_data/header_valid_unix.ply");

  char c;
  std::string base_string;
  while (input.get(c)) {
    base_string += c;
  }

  for (size_t i = 4; i < base_string.size(); i++) {
    std::string string_copy = base_string;
    string_copy[i] = '\t';
    std::stringstream stream(string_copy);
    auto result = plyodine::internal::ParseHeader(stream);
    EXPECT_EQ("The input contained an invalid character",
              result.error().Message());
  }
}
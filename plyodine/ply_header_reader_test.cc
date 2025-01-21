#include "plyodine/ply_header_reader.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace plyodine {
namespace {

using ::bazel::tools::cpp::runfiles::Runfiles;
using ::testing::StartsWith;

std::ifstream OpenRunfile(const std::string& path) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  return std::ifstream(runfiles->Rlocation(path),
                       std::ios::in | std::ios::binary);
}

TEST(ReadPlyHeader, DefaultErrorCondition) {
  std::stringstream input(std::ios::in | std::ios::binary);
  input.clear(std::ios::badbit);

  auto result = ReadPlyHeader(input);
  const std::error_category& error_catgegory = result.error().category();

  EXPECT_NE(error_catgegory.default_error_condition(0),
            std::errc::invalid_argument);
  for (int i = 1; i <= 24; i++) {
    EXPECT_EQ(error_catgegory.default_error_condition(i),
              std::errc::invalid_argument);
  }
  EXPECT_NE(error_catgegory.default_error_condition(25),
            std::errc::invalid_argument);
}

TEST(ReadPlyHeader, BadStream) {
  std::stringstream input(std::ios::in | std::ios::binary);
  input.clear(std::ios::badbit);
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("The stream was not in 'good' state", result.error().message());
}

TEST(ReadPlyHeader, BadMagicStringMac) {
  const std::string magic_string = "ply\r";
  for (size_t i = 0; i < magic_string.length(); i++) {
    std::stringstream stream(magic_string.substr(0u, i),
                             std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(stream);
    EXPECT_EQ("The first line of the header must contain only 'ply'",
              result.error().message());
  }

  std::stringstream stream(magic_string, std::ios::in | std::ios::binary);
  auto result = ReadPlyHeader(stream);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, BadMagicStringUnix) {
  const std::string magic_string = "ply\n";
  for (size_t i = 0; i < magic_string.length(); i++) {
    std::stringstream stream(magic_string.substr(0u, i),
                             std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(stream);
    EXPECT_EQ("The first line of the header must contain only 'ply'",
              result.error().message());
  }

  std::stringstream stream(magic_string, std::ios::in | std::ios::binary);
  auto result = ReadPlyHeader(stream);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, BadMagicStringWindows) {
  const std::string magic_string = "ply\r\n";
  for (size_t i = 0; i < magic_string.length() - 1; i++) {
    std::stringstream stream(magic_string.substr(0u, i),
                             std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(stream);
    EXPECT_EQ("The first line of the header must contain only 'ply'",
              result.error().message());
  }

  std::stringstream stream(magic_string, std::ios::in | std::ios::binary);
  auto result = ReadPlyHeader(stream);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, MismatchedLineEndings) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_mismatched_endings.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("The input contained mismatched line endings",
            result.error().message());
}

TEST(ReadPlyHeader, NoFileFormat) {
  std::stringstream input("ply\nformat", std::ios::in | std::ios::binary);
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, FormatASCII) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_format_ascii.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(PlyHeader::Format::ASCII, result->format);
}

TEST(ReadPlyHeader, FormatBigEndian) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_format_big_endian.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(PlyHeader::Format::BINARY_BIG_ENDIAN, result->format);
}

TEST(ReadPlyHeader, FormatLittleEndian) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_format_little_endian.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(PlyHeader::Format::BINARY_LITTLE_ENDIAN, result->format);
}

TEST(ReadPlyHeader, FormatBad) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_format_bad.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The input specified an invalid format (must be one of 'ascii', "
      "'binary_big_endian', or 'binary_little_endian')",
      result.error().message());
}

TEST(ReadPlyHeader, FormatNoVersion) {
  std::stringstream input("ply\nformat ascii", std::ios::in | std::ios::binary);
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, FormatGoodVersions) {
  std::string good_versions[] = {"1", "1.", "1.0", "01", "0001.", "1.0000"};
  for (const auto& version : good_versions) {
    std::stringstream input("ply\nformat ascii " + version + "\nend_header");
    auto result = ReadPlyHeader(input);
    EXPECT_EQ(1u, result->major_version);
    EXPECT_EQ(0u, result->minor_version);
  }
}

TEST(ReadPlyHeader, FormatBadVersions) {
  std::string bad_versions[] = {"11",  "11.",  "11.0", "2",   "2.",
                                "2.0", "2.00", ".",    ".0",  "0",
                                "-1",  "-1.0", "0.0",  "1..0"};
  for (const auto& version : bad_versions) {
    std::stringstream input("ply\nformat ascii " + version + "\nend_header",
                            std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(input);
    EXPECT_EQ(
        "The input specified an unsupported PLY version (maximum supported "
        "version is '1.0')",
        result.error().message());
  }
}

TEST(ReadPlyHeader, FormatTooLong) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_format_too_long.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The second line of the header must contain only the format specifier "
      "(must follow structure 'format "
      "<ascii|binary_big_endian|binary_little_endian> 1.0')",
      result.error().message());
}

TEST(ReadPlyHeader, ElementNoName) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_name_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "An element declaration was invalid (its line must follow structure "
      "'element <name> <number of instances>')",
      result.error().message());
}

TEST(ReadPlyHeader, ElementNameRepeated) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_name_repeated.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("The input declared two elements with the same name",
            result.error().message());
}

TEST(ReadPlyHeader, ElementCountNone) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_count_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "An element declaration was invalid (its line must follow structure "
      "'element <name> <number of instances>')",
      result.error().message());
}

TEST(ReadPlyHeader, ElementCountBad) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_count_bad.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "An element declaration contained an instance count that could not be "
      "parsed as an integer",
      result.error().message());
}

TEST(ReadPlyHeader, ElementCountNegative) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_count_negative.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_THAT(result.error().message(),
              StartsWith("An element declaration contained an instance count "
                         "that was out of range"));
}

TEST(ReadPlyHeader, ElementCountTooLarge) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_element_count_too_large.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_THAT(result.error().message(),
              StartsWith("An element declaration contained an instance count "
                         "that was out of range"));
}

TEST(ReadPlyHeader, ElementCountTooMany) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_element_too_many.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "An element declaration was invalid (its line must follow structure "
      "'element <name> <number of instances>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyTypes) {
  std::string types[16] = {"char",  "uchar",  "short",   "ushort",
                           "int",   "uint",   "float",   "double",
                           "int8",  "uint8",  "int16",   "uint16",
                           "int32", "uint32", "float32", "float64"};
  PlyHeader::Property::Type parsed_types[16] = {
      PlyHeader::Property::Type::CHAR,  PlyHeader::Property::Type::UCHAR,
      PlyHeader::Property::Type::SHORT, PlyHeader::Property::Type::USHORT,
      PlyHeader::Property::Type::INT,   PlyHeader::Property::Type::UINT,
      PlyHeader::Property::Type::FLOAT, PlyHeader::Property::Type::DOUBLE,
      PlyHeader::Property::Type::CHAR,  PlyHeader::Property::Type::UCHAR,
      PlyHeader::Property::Type::SHORT, PlyHeader::Property::Type::USHORT,
      PlyHeader::Property::Type::INT,   PlyHeader::Property::Type::UINT,
      PlyHeader::Property::Type::FLOAT, PlyHeader::Property::Type::DOUBLE};

  std::string base = "ply\nformat ascii 1.0\nelement vertex 1\n";
  for (size_t i = 0; i < 8; i++) {
    std::stringstream input(
        "ply\nformat ascii 1.0\nelement vertex 1\nproperty " + types[i] +
            " name\nend_header",
        std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(input);
    EXPECT_EQ(parsed_types[i],
              result->elements.at(0).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(0).list_type);
  }
}

TEST(ReadPlyHeader, PropertyNameNone) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_property_name_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property declaration was invalid (its line must follow structure "
      "'property <char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyNameDuplicated) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_name_duplicated.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The input declared two properties of an element with the same name",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyTypeNone) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_property_type_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property declaration was invalid (its line must follow structure "
      "'property [(list <char|uchar|short|ushort|int|uint>)] "
      "<char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyTypeBad) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_property_type_bad.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property declaration specified an invalid type (must be one of "
      "'char', 'uchar', 'short', 'ushort', 'int', 'uint', 'float', or "
      "'double')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyTooMany) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_property_too_many.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property declaration was invalid (its line must follow structure "
      "'property <char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListNameNone) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_name_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property list declaration was invalid (its line must follow structure "
      "'property list <char|uchar|short|ushort|int|uint> "
      "<char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListNameDuplicated) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_name_duplicated.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The input declared two properties of an element with the same name",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListDataTypeNone) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_data_type_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property list declaration was invalid (its line must follow structure "
      "'property list <char|uchar|short|ushort|int|uint> "
      "<char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListDataTypeBad) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_data_type_bad.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property list declaration specified an invalid data type (must be one "
      "of 'char', 'uchar', 'short', 'ushort', 'int', 'uint', 'float', or "
      "'double')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListListTypeNone) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_list_type_none.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property list declaration was invalid (its line must follow structure "
      "'property list <char|uchar|short|ushort|int|uint> "
      "<char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListListTypeBad) {
  std::ifstream input = OpenRunfile(
      "_main/plyodine/test_data/header_property_list_list_type_bad.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property list declaration specified an invalid size type (must be one "
      "of 'char', 'uchar', 'short', 'ushort', 'int', or 'uint')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListTooMany) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_property_list_too_many.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "A property declaration was invalid (its line must follow structure "
      "'property <char|uchar|short|ushort|int|uint|float|double> <name>')",
      result.error().message());
}

TEST(ReadPlyHeader, PropertyListTypes) {
  std::string types[16] = {"char",  "uchar",  "short",   "ushort",
                           "int",   "uint",   "float",   "double",
                           "int8",  "uint8",  "int16",   "uint16",
                           "int32", "uint32", "float32", "float64"};
  PlyHeader::Property::Type parsed_types[16] = {
      PlyHeader::Property::Type::CHAR,  PlyHeader::Property::Type::UCHAR,
      PlyHeader::Property::Type::SHORT, PlyHeader::Property::Type::USHORT,
      PlyHeader::Property::Type::INT,   PlyHeader::Property::Type::UINT,
      PlyHeader::Property::Type::FLOAT, PlyHeader::Property::Type::DOUBLE,
      PlyHeader::Property::Type::CHAR,  PlyHeader::Property::Type::UCHAR,
      PlyHeader::Property::Type::SHORT, PlyHeader::Property::Type::USHORT,
      PlyHeader::Property::Type::INT,   PlyHeader::Property::Type::UINT,
      PlyHeader::Property::Type::FLOAT, PlyHeader::Property::Type::DOUBLE};

  std::string base = "ply\nformat ascii 1.0\nelement vertex 1\n";
  for (size_t i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      std::stringstream input(
          "ply\nformat ascii 1.0\nelement vertex 1\nproperty list " + types[i] +
              " " + types[j] + " name\nend_header",
          std::ios::in | std::ios::binary);

      auto result = ReadPlyHeader(input);
      if (parsed_types[i] == PlyHeader::Property::Type::FLOAT) {
        EXPECT_EQ(
            "A property list declaration specified 'float' as its size type "
            "(must be one of 'char', 'uchar', 'short', 'ushort', 'int', or "
            "'uint')",
            result.error().message());
      } else if (parsed_types[i] == PlyHeader::Property::Type::DOUBLE) {
        EXPECT_EQ(
            "A property list declaration specified 'double' as its size type "
            "(must be one of 'char', 'uchar', 'short', 'ushort', 'int', or "
            "'uint')",
            result.error().message());
      } else {
        EXPECT_EQ(parsed_types[i],
                  result->elements.at(0).properties.at(0).list_type.value());
        EXPECT_EQ(parsed_types[j],
                  result->elements.at(0).properties.at(0).data_type);
      }
    }
  }
}

TEST(ReadPlyHeader, LooseProperty) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_loose_property.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(
      "The input declared a property before its first element declaration",
      result.error().message());
}

TEST(ReadPlyHeader, CommentAllowsSpaces) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_comment_allows_spaces.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(" comment with multiple  spaces  ", result->comments.at(0));
}

TEST(ReadPlyHeader, CommentEmpty) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_comment_empty.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_TRUE(result->comments.at(0).empty());
}

TEST(ReadPlyHeader, ObjInfoAllowsSpaces) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_obj_info_allows_spaces.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ(" comment with multiple  spaces  ", result->object_info.at(0));
}

TEST(ReadPlyHeader, ObjInfoEmpty) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_obj_info_empty.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_TRUE(result->object_info.at(0).empty());
}

TEST(ReadPlyHeader, EndTooMany) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_end_too_many.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("The last line of the header must contain only 'end_header'",
            result.error().message());
}

TEST(ReadPlyHeader, EmptyLine) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_empty_line.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("A line in the header was empty", result.error().message());
}

TEST(ReadPlyHeader, InvalidKeyword) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_invalid_keyword.ply");
  auto result = ReadPlyHeader(input);
  EXPECT_EQ("A line in the header contained an invalid keyword",
            result.error().message());
}

TEST(ReadPlyHeader, Valid) {
  std::string files[] = {
      "_main/plyodine/test_data/header_valid_mac.ply",
      "_main/plyodine/test_data/header_valid_unix.ply",
      "_main/plyodine/test_data/header_valid_windows.ply",
      "_main/plyodine/test_data/header_valid_with_space.ply"};
  std::string line_endings[] = {"\r", "\n", "\r\n", "\n"};

  for (size_t i = 0; i < 4; i++) {
    std::ifstream input = OpenRunfile(files[i]);
    auto result = ReadPlyHeader(input);
    ASSERT_TRUE(result);

    EXPECT_EQ(PlyHeader::Format::ASCII, result->format);
    EXPECT_EQ(line_endings[i], result->line_ending);
    EXPECT_EQ(1u, result->major_version);
    EXPECT_EQ(0u, result->minor_version);
    EXPECT_EQ(2u, result->comments.size());
    EXPECT_EQ("author: Greg Turk", result->comments.at(0));
    EXPECT_EQ("object: another cube", result->comments.at(1));
    EXPECT_EQ("obj info 0", result->object_info.at(0));
    EXPECT_EQ("obj info 1", result->object_info.at(1));
    EXPECT_EQ(3u, result->elements.size());

    EXPECT_EQ("vertex", result->elements.at(0).name);
    EXPECT_EQ(6u, result->elements.at(0).properties.size());
    EXPECT_EQ("x", result->elements.at(0).properties.at(0).name);
    EXPECT_EQ(PlyHeader::Property::Type::FLOAT,
              result->elements.at(0).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(0).list_type);
    EXPECT_EQ("y", result->elements.at(0).properties.at(1).name);
    EXPECT_EQ(PlyHeader::Property::Type::FLOAT,
              result->elements.at(0).properties.at(1).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(1).list_type);
    EXPECT_EQ("z", result->elements.at(0).properties.at(2).name);
    EXPECT_EQ(PlyHeader::Property::Type::FLOAT,
              result->elements.at(0).properties.at(2).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(2).list_type);
    EXPECT_EQ("red", result->elements.at(0).properties.at(3).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(0).properties.at(3).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(3).list_type);
    EXPECT_EQ("green", result->elements.at(0).properties.at(4).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(0).properties.at(4).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(4).list_type);
    EXPECT_EQ("blue", result->elements.at(0).properties.at(5).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(0).properties.at(5).data_type);
    EXPECT_FALSE(result->elements.at(0).properties.at(5).list_type);

    EXPECT_EQ("face", result->elements.at(1).name);
    EXPECT_EQ(1u, result->elements.at(1).properties.size());
    EXPECT_EQ("vertex_index", result->elements.at(1).properties.at(0).name);
    EXPECT_EQ(PlyHeader::Property::Type::INT,
              result->elements.at(1).properties.at(0).data_type);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              *result->elements.at(1).properties.at(0).list_type);

    EXPECT_EQ("edge", result->elements.at(2).name);
    EXPECT_EQ(5u, result->elements.at(2).properties.size());
    EXPECT_EQ("vertex1", result->elements.at(2).properties.at(0).name);
    EXPECT_EQ(PlyHeader::Property::Type::INT,
              result->elements.at(2).properties.at(0).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(0).list_type);
    EXPECT_EQ("vertex2", result->elements.at(2).properties.at(1).name);
    EXPECT_EQ(PlyHeader::Property::Type::INT,
              result->elements.at(2).properties.at(1).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(1).list_type);
    EXPECT_EQ("red", result->elements.at(2).properties.at(2).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(2).properties.at(2).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(2).list_type);
    EXPECT_EQ("green", result->elements.at(2).properties.at(3).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(2).properties.at(3).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(3).list_type);
    EXPECT_EQ("blue", result->elements.at(2).properties.at(4).name);
    EXPECT_EQ(PlyHeader::Property::Type::UCHAR,
              result->elements.at(2).properties.at(4).data_type);
    EXPECT_FALSE(result->elements.at(2).properties.at(4).list_type);
  }
}

TEST(ReadPlyHeader, InvalidCharacters) {
  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/header_valid_unix.ply");

  char c;
  std::string base_string;
  while (input.get(c)) {
    base_string += c;
  }

  for (size_t i = 4; i < base_string.size(); i++) {
    std::string string_copy = base_string;
    string_copy[i] = '\v';
    std::stringstream stream(string_copy, std::ios::in | std::ios::binary);
    auto result = ReadPlyHeader(stream);
    EXPECT_EQ(
        "The input contained an invalid character in its header (each line "
        "must contain only printable ASCII characters)",
        result.error().message());
  }
}

}  // namespace
}  // namespace plyodine
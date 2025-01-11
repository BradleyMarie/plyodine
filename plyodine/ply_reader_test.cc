#include "plyodine/ply_reader.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace plyodine {
namespace {

using ::bazel::tools::cpp::runfiles::Runfiles;
using ::testing::_;
using ::testing::InSequence;
using ::testing::IsEmpty;
using ::testing::Return;

enum class PropertyType {
  CHAR = 0u,
  CHAR_LIST = 1u,
  UCHAR = 2u,
  UCHAR_LIST = 3u,
  SHORT = 4u,
  SHORT_LIST = 5u,
  USHORT = 6u,
  USHORT_LIST = 7u,
  INT = 8u,
  INT_LIST = 9u,
  UINT = 10u,
  UINT_LIST = 11u,
  FLOAT = 12u,
  FLOAT_LIST = 13u,
  DOUBLE = 14u,
  DOUBLE_LIST = 15u
};

class MockPlyReader final : public PlyReader {
 public:
  bool initialize_callbacks = true;

  MOCK_METHOD(std::error_code, StartImpl,
              ((const std::map<
                   std::string,
                   std::pair<uintmax_t, std::map<std::string, PropertyType>>>&),
               const std::vector<std::string>&,
               const std::vector<std::string>&),
              ());

  MOCK_METHOD(std::error_code, HandleInt8,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int8_t));
  MOCK_METHOD(std::error_code, HandleInt8List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int8_t>));

  MOCK_METHOD(std::error_code, HandleUInt8,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint8_t));
  MOCK_METHOD(std::error_code, HandleUInt8List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint8_t>));

  MOCK_METHOD(std::error_code, HandleInt16,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int16_t));
  MOCK_METHOD(std::error_code, HandleInt16List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int16_t>));

  MOCK_METHOD(std::error_code, HandleUInt16,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint16_t));
  MOCK_METHOD(std::error_code, HandleUInt16List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint16_t>));

  MOCK_METHOD(std::error_code, HandleInt32,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int32_t));
  MOCK_METHOD(std::error_code, HandleInt32List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int32_t>));

  MOCK_METHOD(std::error_code, HandleUInt32,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint32_t));
  MOCK_METHOD(std::error_code, HandleUInt32List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint32_t>));

  MOCK_METHOD(std::error_code, HandleFloat,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, float));
  MOCK_METHOD(std::error_code, HandleFloatList,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const float>));

  MOCK_METHOD(std::error_code, HandleDouble,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, double));
  MOCK_METHOD(std::error_code, HandleDoubleList,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const double>));

  std::error_code Start(
      const std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      const std::vector<std::string>& comments,
      const std::vector<std::string>& object_info) override {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties;
    for (const auto& element : callbacks) {
      for (const auto& property : element.second) {
        properties[element.first].first =
            num_element_instances.at(element.first);
        properties[element.first].second[property.first] =
            static_cast<PropertyType>(property.second.index());
      }
    }

    if (std::error_code error = StartImpl(properties, comments, object_info);
        error) {
      return error;
    }

    std::map<std::string, std::map<std::string, PlyReader::PropertyCallback>>
        result;
    if (!initialize_callbacks) {
      return std::error_code();
    }

    for (const auto& element : properties) {
      for (const auto& property : element.second.second) {
        switch (property.second) {
          case PropertyType::CHAR:
            callbacks[element.first][property.first] =
                PlyReader::Int8PropertyCallback(&MockPlyReader::HandleInt8);
            break;
          case PropertyType::CHAR_LIST:
            callbacks[element.first][property.first] =
                PlyReader::Int8PropertyListCallback(
                    &MockPlyReader::HandleInt8List);
            break;
          case PropertyType::UCHAR:
            callbacks[element.first][property.first] =
                PlyReader::UInt8PropertyCallback(&MockPlyReader::HandleUInt8);
            break;
          case PropertyType::UCHAR_LIST:
            callbacks[element.first][property.first] =
                PlyReader::UInt8PropertyListCallback(
                    &MockPlyReader::HandleUInt8List);
            break;
          case PropertyType::SHORT:
            callbacks[element.first][property.first] =
                PlyReader::Int16PropertyCallback(&MockPlyReader::HandleInt16);
            break;
          case PropertyType::SHORT_LIST:
            callbacks[element.first][property.first] =
                PlyReader::Int16PropertyListCallback(
                    &MockPlyReader::HandleInt16List);
            break;
          case PropertyType::USHORT:
            callbacks[element.first][property.first] =
                PlyReader::UInt16PropertyCallback(&MockPlyReader::HandleUInt16);
            break;
          case PropertyType::USHORT_LIST:
            callbacks[element.first][property.first] =
                PlyReader::UInt16PropertyListCallback(
                    &MockPlyReader::HandleUInt16List);
            break;
          case PropertyType::INT:
            callbacks[element.first][property.first] =
                PlyReader::Int32PropertyCallback(&MockPlyReader::HandleInt32);
            break;
          case PropertyType::INT_LIST:
            callbacks[element.first][property.first] =
                PlyReader::Int32PropertyListCallback(
                    &MockPlyReader::HandleInt32List);
            break;
          case PropertyType::UINT:
            callbacks[element.first][property.first] =
                PlyReader::UInt32PropertyCallback(&MockPlyReader::HandleUInt32);
            break;
          case PropertyType::UINT_LIST:
            callbacks[element.first][property.first] =
                PlyReader::UInt32PropertyListCallback(
                    &MockPlyReader::HandleUInt32List);
            break;
          case PropertyType::FLOAT:
            callbacks[element.first][property.first] =
                PlyReader::FloatPropertyCallback(&MockPlyReader::HandleFloat);
            break;
          case PropertyType::FLOAT_LIST:
            callbacks[element.first][property.first] =
                PlyReader::FloatPropertyListCallback(
                    &MockPlyReader::HandleFloatList);
            break;
          case PropertyType::DOUBLE:
            callbacks[element.first][property.first] =
                PlyReader::DoublePropertyCallback(&MockPlyReader::HandleDouble);
            break;
          case PropertyType::DOUBLE_LIST:
            callbacks[element.first][property.first] =
                PlyReader::DoublePropertyListCallback(
                    &MockPlyReader::HandleDoubleList);
            break;
        }
      }
    }

    return std::error_code();
  }
};

MATCHER_P(PropertiesAre, properties, "") {
  if (properties.size() != arg.size()) {
    return false;
  }

  for (const auto& [element_name, element_properties] : properties) {
    if (!arg.contains(element_name) ||
        arg.at(element_name).first != element_properties.first ||
        arg.at(element_name).second.size() !=
            element_properties.second.size()) {
      return false;
    }

    for (const auto& [property_name, property_type] :
         arg.at(element_name).second) {
      if (!element_properties.second.contains(property_name) ||
          element_properties.second.at(property_name) != property_type) {
        return false;
      }
    }
  }

  return true;
}

MATCHER_P(ValuesAre, values, "") {
  if (values.size() != arg.size()) {
    return false;
  }

  for (size_t i = 0; i < values.size(); i++) {
    if (values[i] != arg[i]) {
      return false;
    }
  }

  return true;
}

std::ifstream OpenRunfile(const std::string& path) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  return std::ifstream(runfiles->Rlocation(path),
                       std::ios::in | std::ios::binary);
}

void ExpectError(std::istream& stream) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _))
      .WillRepeatedly(Return(std::error_code()));

  EXPECT_NE(0, reader.ReadFrom(stream).value());
}

void RunReadErrorTest(const std::string& file_name,
                      size_t num_bytes = std::numeric_limits<size_t>::max()) {
  std::ifstream input = OpenRunfile(file_name);

  char c;
  std::string base_string;
  while (input.get(c)) {
    base_string += c;
  }

  if (num_bytes < base_string.size()) {
    base_string.resize(num_bytes);
  }

  for (size_t i = 0; i < base_string.size(); i++) {
    std::string string_copy = base_string;
    string_copy.resize(i);
    std::stringstream stream(string_copy, std::ios::in | std::ios::binary);
    ExpectError(stream);
  }
}

TEST(Validate, DefaultErrorCondition) {
  MockPlyReader reader;
  std::stringstream input(std::ios::in | std::ios::binary);
  input.clear(std::ios::badbit);

  const std::error_category& error_catgegory =
      reader.ReadFrom(input).category();

  EXPECT_NE(error_catgegory.default_error_condition(0),
            std::errc::invalid_argument);
  for (int i = 1; i <= 12; i++) {
    EXPECT_EQ(error_catgegory.default_error_condition(i),
              std::errc::invalid_argument);
  }
  EXPECT_NE(error_catgegory.default_error_condition(13),
            std::errc::invalid_argument);
}

TEST(Validate, BadStream) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::stringstream input(std::ios::in | std::ios::binary);
  input.clear(std::ios::badbit);

  EXPECT_EQ("Input stream must be in good state",
            reader.ReadFrom(input).message());
}

TEST(Error, BadHeader) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/header_format_bad.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ(
      "Format must be one of ascii, binary_big_endian, or binary_little_endian",
      result.message());
}

TEST(Header, StartFails) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(IsEmpty(), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code(1, std::generic_category())));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(reader.ReadFrom(stream).value(), 1);
}

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(IsEmpty(), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, MismatchedLineEndings) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::CHAR}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_mismatched_line_endings.ply");
  EXPECT_EQ(reader.ReadFrom(stream).message(),
            "The input contained mismatched line endings");
}

TEST(ASCII, InvalidCharacter) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::CHAR}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_invalid_character.ply");
  EXPECT_EQ(reader.ReadFrom(stream).message(),
            "The input contained an invalid character");
}

TEST(ASCII, ListMissingEntries) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_missing_entries.ply");
  EXPECT_EQ(reader.ReadFrom(stream).message(),
            "The input contained an element with too few tokens");
}

TEST(ASCII, MissingElement) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"l", PropertyType::CHAR}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "l", 0, 0, 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_missing_element.ply");
  EXPECT_EQ(reader.ReadFrom(stream).message(), "Unexpected EOF");
}

TEST(ASCII, ExtraWhitespace) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {
          {"vertex",
           {2u, {{"a", PropertyType::CHAR}, {"b", PropertyType::CHAR}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty_token.ply");
  EXPECT_EQ(
      reader.ReadFrom(stream).message(),
      "Non-comment ASCII lines may only contain a single space between tokens");
}

TEST(ASCII, ListSizeTooLarge) {
  auto impl = [](const std::string& name) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader,
                StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
        .Times(1)
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).message(),
              "The input contained a property list size that was out of range");
  };

  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_int8.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_int16.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_int32.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_uint8.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_uint16.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_too_large_uint32.ply");
}

TEST(ASCII, ListSizeBad) {
  auto impl = [](const std::string& name) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader,
                StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
        .Times(1)
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).message(),
              "The input contained a property list size that failed to parse");
  };

  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_int8.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_int16.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_int32.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_uint8.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_uint16.ply");
  impl("_main/plyodine/test_data/ply_ascii_list_sizes_bad_uint32.ply");
}

TEST(ASCII, EntryBad) {
  auto impl = [](const std::string& name, PropertyType type) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", type}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader,
                StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
        .Times(1)
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).message(),
              "The input contained a property entry that failed to parse");
  };

  impl("_main/plyodine/test_data/ply_ascii_entry_bad_double.ply",
       PropertyType::DOUBLE);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_float.ply",
       PropertyType::FLOAT);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int8.ply",
       PropertyType::CHAR);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int16.ply",
       PropertyType::SHORT);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int32.ply",
       PropertyType::INT);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint8.ply",
       PropertyType::UCHAR);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint16.ply",
       PropertyType::USHORT);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint32.ply",
       PropertyType::UINT);
}

TEST(ASCII, EntryTooBig) {
  auto impl = [](const std::string& name, PropertyType type) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", type}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader,
                StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
        .Times(1)
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).message(),
              "The input contained a property entry that was out of range");
  };

  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_double.ply",
       PropertyType::DOUBLE);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_float.ply",
       PropertyType::FLOAT);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int8.ply",
       PropertyType::CHAR);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int16.ply",
       PropertyType::SHORT);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int32.ply",
       PropertyType::INT);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint8.ply",
       PropertyType::UCHAR);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint16.ply",
       PropertyType::USHORT);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint32.ply",
       PropertyType::UINT);
}

TEST(ASCII, UnusedTokens) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::CHAR}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0u, 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_unused_tokens.ply");
  EXPECT_EQ(reader.ReadFrom(stream).message(),
            "The input contained an element with unused tokens");
}

TEST(ASCII, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index, size_t index) -> std::error_code {
      if (case_index == index) {
        return std::error_code(1, std::generic_category());
      }

      return std::error_code();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(_, _, _)).WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).value(), 1);
  };

  impl(0u);
  impl(1u);
  impl(2u);
  impl(3u);
  impl(4u);
  impl(5u);
  impl(6u);
  impl(7u);
  impl(8u);
  impl(9u);
  impl(10u);
  impl(11u);
  impl(12u);
  impl(13u);
  impl(14u);
  impl(15u);
}

TEST(ASCII, WithUIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_list_sizes.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, WithIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(ASCII, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(ASCII, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(BigEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(IsEmpty(), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_empty.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithDataError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_data.ply");
}

TEST(BigEndian, WithUIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithUIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_list_sizes.ply", 1000u);
}

TEST(BigEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index, size_t index) -> std::error_code {
      if (case_index == index) {
        return std::error_code(1, std::generic_category());
      }

      return std::error_code();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(_, _, _)).WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).value(), 1);
  };

  impl(0u);
  impl(1u);
  impl(2u);
  impl(3u);
  impl(4u);
  impl(5u);
  impl(6u);
  impl(7u);
  impl(8u);
  impl(9u);
  impl(10u);
  impl(11u);
  impl(12u);
  impl(13u);
  impl(14u);
  impl(15u);
}

TEST(BigEndian, WithIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_list_sizes_signed.ply",
                   1000u);
}

TEST(BigEndian, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(BigEndian, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(BigEndian, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(LittleEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(IsEmpty(), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_empty.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::CHAR},
                       {"b", PropertyType::UCHAR},
                       {"c", PropertyType::SHORT},
                       {"d", PropertyType::USHORT},
                       {"e", PropertyType::INT},
                       {"f", PropertyType::UINT},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::CHAR_LIST},
                       {"b", PropertyType::UCHAR_LIST},
                       {"c", PropertyType::SHORT_LIST},
                       {"d", PropertyType::USHORT_LIST},
                       {"e", PropertyType::INT_LIST},
                       {"f", PropertyType::UINT_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithDataError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_data.ply");
}

TEST(LittleEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index, size_t index) -> std::error_code {
      if (case_index == index) {
        return std::error_code(1, std::generic_category());
      }

      return std::error_code();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(_, _, _)).WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _, _, _, _))
        .WillRepeatedly(Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).value(), 1);
  };

  impl(0u);
  impl(1u);
  impl(2u);
  impl(3u);
  impl(4u);
  impl(5u);
  impl(6u);
  impl(7u);
  impl(8u);
  impl(9u);
  impl(10u);
  impl(11u);
  impl(12u);
  impl(13u);
  impl(14u);
  impl(15u);
}

TEST(LittleEndian, WithUIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithUIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_list_sizes.ply", 1000u);
}

TEST(LittleEndian, WithIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UCHAR_LIST},
                       {"l1", PropertyType::UCHAR_LIST},
                       {"l2", PropertyType::UCHAR_LIST},
                       {"l3", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_signed_list_sizes.ply",
                   1000u);
}

TEST(LittleEndian, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(LittleEndian, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

TEST(LittleEndian, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UCHAR_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader,
              StartImpl(PropertiesAre(properties), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.message());
}

}  // namespace
}  // namespace plyodine
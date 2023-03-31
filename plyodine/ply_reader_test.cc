#include "plyodine/ply_reader.h"

#include <fstream>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

class MockPlyReader final : public plyodine::PlyReader {
 public:
  bool initialize_callbacks = true;

  MOCK_METHOD(
      (std::expected<void, std::string_view>), StartImpl,
      ((const std::unordered_map<
           std::string_view,
           std::pair<uint64_t, std::unordered_map<std::string_view,
                                                  plyodine::Property::Type>>>&),
       std::span<const std::string>, std::span<const std::string>),
      ());

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt8,
              (std::string_view, std::string_view, plyodine::Int8Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt8List,
              (std::string_view, std::string_view, plyodine::Int8PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt8,
              (std::string_view, std::string_view, plyodine::UInt8Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt8List,
              (std::string_view, std::string_view,
               plyodine::UInt8PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt16,
              (std::string_view, std::string_view, plyodine::Int16Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt16List,
              (std::string_view, std::string_view,
               plyodine::Int16PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt16,
              (std::string_view, std::string_view, plyodine::UInt16Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt16List,
              (std::string_view, std::string_view,
               plyodine::UInt16PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt32,
              (std::string_view, std::string_view, plyodine::Int32Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt32List,
              (std::string_view, std::string_view,
               plyodine::Int32PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt32,
              (std::string_view, std::string_view, plyodine::UInt32Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt32List,
              (std::string_view, std::string_view,
               plyodine::UInt32PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleFloat,
              (std::string_view, std::string_view, plyodine::FloatProperty));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleFloatList,
              (std::string_view, std::string_view,
               plyodine::FloatPropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleDouble,
              (std::string_view, std::string_view, plyodine::DoubleProperty));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleDoubleList,
              (std::string_view, std::string_view,
               plyodine::DoublePropertyList));

  std::expected<
      std::unordered_map<
          std::string_view,
          std::unordered_map<std::string_view, plyodine::PlyReader::Callback>>,
      std::string_view>
  Start(const std::unordered_map<
            std::string_view,
            std::pair<uint64_t, std::unordered_map<std::string_view,
                                                   plyodine::Property::Type>>>&
            properties,
        std::span<const std::string> comments,
        std::span<const std::string> object_info) override {
    auto error = StartImpl(properties, comments, object_info);
    if (!error) {
      return std::unexpected(error.error());
    }

    std::unordered_map<
        std::string_view,
        std::unordered_map<std::string_view, plyodine::PlyReader::Callback>>
        result;
    if (!initialize_callbacks) {
      return result;
    }

    for (const auto& element : properties) {
      for (const auto& property : element.second.second) {
        switch (property.second) {
          case plyodine::Property::INT8:
            result[element.first][property.first] =
                plyodine::PlyReader::Int8PropertyCallback(
                    &MockPlyReader::HandleInt8);
            break;
          case plyodine::Property::INT8_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::Int8PropertyListCallback(
                    &MockPlyReader::HandleInt8List);
            break;
          case plyodine::Property::UINT8:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt8PropertyCallback(
                    &MockPlyReader::HandleUInt8);
            break;
          case plyodine::Property::UINT8_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt8PropertyListCallback(
                    &MockPlyReader::HandleUInt8List);
            break;
          case plyodine::Property::INT16:
            result[element.first][property.first] =
                plyodine::PlyReader::Int16PropertyCallback(
                    &MockPlyReader::HandleInt16);
            break;
          case plyodine::Property::INT16_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::Int16PropertyListCallback(
                    &MockPlyReader::HandleInt16List);
            break;
          case plyodine::Property::UINT16:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt16PropertyCallback(
                    &MockPlyReader::HandleUInt16);
            break;
          case plyodine::Property::UINT16_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt16PropertyListCallback(
                    &MockPlyReader::HandleUInt16List);
            break;
          case plyodine::Property::INT32:
            result[element.first][property.first] =
                plyodine::PlyReader::Int32PropertyCallback(
                    &MockPlyReader::HandleInt32);
            break;
          case plyodine::Property::INT32_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::Int32PropertyListCallback(
                    &MockPlyReader::HandleInt32List);
            break;
          case plyodine::Property::UINT32:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt32PropertyCallback(
                    &MockPlyReader::HandleUInt32);
            break;
          case plyodine::Property::UINT32_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::UInt32PropertyListCallback(
                    &MockPlyReader::HandleUInt32List);
            break;
          case plyodine::Property::FLOAT:
            result[element.first][property.first] =
                plyodine::PlyReader::FloatPropertyCallback(
                    &MockPlyReader::HandleFloat);
            break;
          case plyodine::Property::FLOAT_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::FloatPropertyListCallback(
                    &MockPlyReader::HandleFloatList);
            break;
          case plyodine::Property::DOUBLE:
            result[element.first][property.first] =
                plyodine::PlyReader::DoublePropertyCallback(
                    &MockPlyReader::HandleDouble);
            break;
          case plyodine::Property::DOUBLE_LIST:
            result[element.first][property.first] =
                plyodine::PlyReader::DoublePropertyListCallback(
                    &MockPlyReader::HandleDoubleList);
            break;
        }
      }
    }

    return result;
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

void ExpectError(std::istream& stream) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));

  EXPECT_FALSE(reader.ReadFrom(stream));
}

void RunReadErrorTest(const std::string& file_name,
                      size_t num_bytes = std::numeric_limits<size_t>::max()) {
  std::ifstream input(file_name);

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
    std::stringstream stream(string_copy);
    ExpectError(stream);
  }
}

TEST(Error, BadHeader) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/header_format_bad.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ(
      "Format must be one of ascii, binary_big_endian, or binary_little_endian",
      result.error());
}

TEST(Header, StartFails) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::unexpected("Failed")));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
}

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, MismatchedLineEndings) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {2u, {{"a", plyodine::Property::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream(
      "plyodine/test_data/ply_ascii_mismatched_line_endings.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained mismatched line endings");
}

TEST(ASCII, InvalidCharacter) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {2u, {{"a", plyodine::Property::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_invalid_character.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an invalid character");
}

TEST(ASCII, ListMissingEntries) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_list_missing_entries.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with too few tokens");
}

TEST(ASCII, MissingElement) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {2u, {{"l", plyodine::Property::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "l", 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_missing_element.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Unexpected EOF");
}

TEST(ASCII, EmptyToken) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {2u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "a", 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty_token.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an empty token");
}

TEST(ASCII, ListSizeTooLarge) {
  auto impl = [](const std::string& name) {
    std::unordered_map<
        std::string_view,
        std::pair<uint64_t, std::unordered_map<std::string_view,
                                               plyodine::Property::Type>>>
        properties = {
            {"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained a property list size that was out of range");
  };

  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_int8.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_int16.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_int32.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_uint8.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_uint16.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_too_large_uint32.ply");
}

TEST(ASCII, ListSizeBad) {
  auto impl = [](const std::string& name) {
    std::unordered_map<
        std::string_view,
        std::pair<uint64_t, std::unordered_map<std::string_view,
                                               plyodine::Property::Type>>>
        properties = {
            {"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained an unparsable property list size");
  };

  impl("plyodine/test_data/ply_ascii_list_sizes_bad_int8.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_bad_int16.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_bad_int32.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_bad_uint8.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_bad_uint16.ply");
  impl("plyodine/test_data/ply_ascii_list_sizes_bad_uint32.ply");
}

TEST(ASCII, EntryBad) {
  auto impl = [](const std::string& name, plyodine::Property::Type type) {
    std::unordered_map<
        std::string_view,
        std::pair<uint64_t, std::unordered_map<std::string_view,
                                               plyodine::Property::Type>>>
        properties = {{"vertex", {1u, {{"l", type}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained an unparsable property entry");
  };

  impl("plyodine/test_data/ply_ascii_entry_bad_double.ply",
       plyodine::Property::DOUBLE);
  impl("plyodine/test_data/ply_ascii_entry_bad_float.ply",
       plyodine::Property::FLOAT);
  impl("plyodine/test_data/ply_ascii_entry_bad_int8.ply",
       plyodine::Property::INT8);
  impl("plyodine/test_data/ply_ascii_entry_bad_int16.ply",
       plyodine::Property::INT16);
  impl("plyodine/test_data/ply_ascii_entry_bad_int32.ply",
       plyodine::Property::INT32);
  impl("plyodine/test_data/ply_ascii_entry_bad_uint8.ply",
       plyodine::Property::UINT8);
  impl("plyodine/test_data/ply_ascii_entry_bad_uint16.ply",
       plyodine::Property::UINT16);
  impl("plyodine/test_data/ply_ascii_entry_bad_uint32.ply",
       plyodine::Property::UINT32);
}

TEST(ASCII, EntryTooBig) {
  auto impl = [](const std::string& name, plyodine::Property::Type type) {
    std::unordered_map<
        std::string_view,
        std::pair<uint64_t, std::unordered_map<std::string_view,
                                               plyodine::Property::Type>>>
        properties = {{"vertex", {1u, {{"l", type}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained a property entry that was out of range");
  };

  impl("plyodine/test_data/ply_ascii_entry_too_large_double.ply",
       plyodine::Property::DOUBLE);
  impl("plyodine/test_data/ply_ascii_entry_too_large_float.ply",
       plyodine::Property::FLOAT);
  impl("plyodine/test_data/ply_ascii_entry_too_large_int8.ply",
       plyodine::Property::INT8);
  impl("plyodine/test_data/ply_ascii_entry_too_large_int16.ply",
       plyodine::Property::INT16);
  impl("plyodine/test_data/ply_ascii_entry_too_large_int32.ply",
       plyodine::Property::INT32);
  impl("plyodine/test_data/ply_ascii_entry_too_large_uint8.ply",
       plyodine::Property::UINT8);
  impl("plyodine/test_data/ply_ascii_entry_too_large_uint16.ply",
       plyodine::Property::UINT16);
  impl("plyodine/test_data/ply_ascii_entry_too_large_uint32.ply",
       plyodine::Property::UINT32);
}

TEST(ASCII, UnusedTokens) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {2u, {{"a", plyodine::Property::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "a", 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_unused_tokens.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with unused tokens");
}

TEST(ASCII, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt16List("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt32List("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_ascii_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithDataSkipAll) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result =
        [](size_t case_index,
           size_t index) -> std::expected<void, std::string_view> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string_view>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream("plyodine/test_data/ply_ascii_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
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
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_ascii_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithIntListSizes) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_ascii_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithNegativeInt8ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_ascii_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(ASCII, WithNegativeInt16ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_ascii_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(ASCII, WithNegativeInt32ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_ascii_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_big_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt16List("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt32List("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_big_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithDataSkipAll) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_big_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithDataError) {
  RunReadErrorTest("plyodine/test_data/ply_big_data.ply");
}

TEST(BigEndian, WithUIntListSizes) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_big_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithUIntListSizesError) {
  RunReadErrorTest("plyodine/test_data/ply_big_list_sizes.ply", 1000u);
}

TEST(BigEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result =
        [](size_t case_index,
           size_t index) -> std::expected<void, std::string_view> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string_view>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream("plyodine/test_data/ply_big_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
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
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_big_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithIntListSizesError) {
  RunReadErrorTest("plyodine/test_data/ply_big_list_sizes_signed.ply", 1000u);
}

TEST(BigEndian, WithNegativeInt8ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_big_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, WithNegativeInt16ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_big_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, WithNegativeInt32ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_big_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_little_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt16List("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt32List("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_little_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithDataSkipAll) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", plyodine::Property::INT8},
                       {"b", plyodine::Property::UINT8},
                       {"c", plyodine::Property::INT16},
                       {"d", plyodine::Property::UINT16},
                       {"e", plyodine::Property::INT32},
                       {"f", plyodine::Property::UINT32},
                       {"g", plyodine::Property::FLOAT},
                       {"h", plyodine::Property::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", plyodine::Property::INT8_LIST},
                       {"b", plyodine::Property::UINT8_LIST},
                       {"c", plyodine::Property::INT16_LIST},
                       {"d", plyodine::Property::UINT16_LIST},
                       {"e", plyodine::Property::INT32_LIST},
                       {"f", plyodine::Property::UINT32_LIST},
                       {"g", plyodine::Property::FLOAT_LIST},
                       {"h", plyodine::Property::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_little_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithDataError) {
  RunReadErrorTest("plyodine/test_data/ply_little_data.ply");
}

TEST(LittleEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result =
        [](size_t case_index,
           size_t index) -> std::expected<void, std::string_view> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string_view>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream("plyodine/test_data/ply_little_data.ply");
    EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
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
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_little_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithUIntListSizesError) {
  RunReadErrorTest("plyodine/test_data/ply_little_list_sizes.ply", 1000u);
}

TEST(LittleEndian, WithIntListSizes) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", plyodine::Property::UINT8_LIST},
                       {"l1", plyodine::Property::UINT8_LIST},
                       {"l2", plyodine::Property::UINT8_LIST},
                       {"l3", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_little_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithIntListSizesError) {
  RunReadErrorTest("plyodine/test_data/ply_little_signed_list_sizes.ply",
                   1000u);
}

TEST(LittleEndian, WithNegativeInt8ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_little_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, WithNegativeInt16ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_little_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, WithNegativeInt32ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t,
                std::unordered_map<std::string_view, plyodine::Property::Type>>>
      properties = {{"vertex", {1u, {{"l", plyodine::Property::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_little_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}
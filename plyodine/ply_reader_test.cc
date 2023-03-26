#include "plyodine/ply_reader.h"

#include <fstream>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

class MockPlyReader final : public plyodine::PlyReader {
 public:
  MOCK_METHOD(
      (std::expected<void, std::string_view>), Start,
      ((const std::unordered_map<
           std::string_view,
           std::pair<uint64_t,
                     std::unordered_map<
                         std::string_view,
                         std::pair<size_t, plyodine::Property::Type>>>>&),
       std::span<const std::string>, std::span<const std::string>),
      (override));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt8,
              (std::string_view, std::string_view, size_t,
               plyodine::Int8Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt8List,
              (std::string_view, std::string_view, size_t,
               plyodine::Int8PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt8,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt8Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt8List,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt8PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt16,
              (std::string_view, std::string_view, size_t,
               plyodine::Int16Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt16List,
              (std::string_view, std::string_view, size_t,
               plyodine::Int16PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt16,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt16Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt16List,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt16PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt32,
              (std::string_view, std::string_view, size_t,
               plyodine::Int32Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleInt32List,
              (std::string_view, std::string_view, size_t,
               plyodine::Int32PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt32,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt32Property));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleUInt32List,
              (std::string_view, std::string_view, size_t,
               plyodine::UInt32PropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleFloat,
              (std::string_view, std::string_view, size_t,
               plyodine::FloatProperty));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleFloatList,
              (std::string_view, std::string_view, size_t,
               plyodine::FloatPropertyList));

  MOCK_METHOD((std::expected<void, std::string_view>), HandleDouble,
              (std::string_view, std::string_view, size_t,
               plyodine::DoubleProperty));
  MOCK_METHOD((std::expected<void, std::string_view>), HandleDoubleList,
              (std::string_view, std::string_view, size_t,
               plyodine::DoublePropertyList));

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int8Property value) override {
    return HandleInt8(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int8PropertyList values) override {
    return HandleInt8List(element_name, property_name, property_index, values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt8Property value) override {
    return HandleUInt8(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt8PropertyList values) override {
    return HandleUInt8List(element_name, property_name, property_index, values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int16Property value) override {
    return HandleInt16(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int16PropertyList values) override {
    return HandleInt16List(element_name, property_name, property_index, values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt16Property value) override {
    return HandleUInt16(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt16PropertyList values) override {
    return HandleUInt16List(element_name, property_name, property_index,
                            values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int32Property value) override {
    return HandleInt32(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::Int32PropertyList values) override {
    return HandleInt32List(element_name, property_name, property_index, values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt32Property value) override {
    return HandleUInt32(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::UInt32PropertyList values) override {
    return HandleUInt32List(element_name, property_name, property_index,
                            values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::FloatProperty value) override {
    return HandleFloat(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::FloatPropertyList values) override {
    return HandleFloatList(element_name, property_name, property_index, values);
  }

  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::DoubleProperty value) override {
    return HandleDouble(element_name, property_name, property_index, value);
  }
  std::expected<void, std::string_view> Handle(
      std::string_view element_name, std::string_view property_name,
      size_t property_index, plyodine::DoublePropertyList values) override {
    return HandleDoubleList(element_name, property_name, property_index,
                            values);
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

    for (const auto& [property_name, property_id_and_type] :
         arg.at(element_name).second) {
      if (!element_properties.second.contains(property_name) ||
          element_properties.second.at(property_name).first !=
              property_id_and_type.first ||
          element_properties.second.at(property_name).second !=
              property_id_and_type.second) {
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
  EXPECT_CALL(reader, Start(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
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
  EXPECT_CALL(reader, Start(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/header_format_bad.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ(
      "Format must be one of ascii, binary_big_endian, or binary_little_endian",
      result.error());
}

TEST(Header, StartFails) {
  MockPlyReader reader;
  EXPECT_CALL(reader,
              Start(testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::unexpected("Failed")));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
}

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader,
              Start(testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, MismatchedLineEndings) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {2u, {{"a", std::make_pair(0, plyodine::Property::INT8)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream(
      "plyodine/test_data/ply_ascii_mismatched_line_endings.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained mismatched line endings");
}

TEST(ASCII, InvalidCharacter) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {2u, {{"a", std::make_pair(0, plyodine::Property::INT8)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_invalid_character.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an invalid character");
}

TEST(ASCII, ListMissingEntries) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_list_missing_entries.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with too few tokens");
}

TEST(ASCII, MissingElement) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {2u, {{"l", std::make_pair(0, plyodine::Property::INT8)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "l", 0, 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_missing_element.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Unexpected EOF");
}

TEST(ASCII, EmptyToken) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {{"vertex",
                     {2u,
                      {{"a", std::make_pair(0, plyodine::Property::INT8)},
                       {"b", std::make_pair(1, plyodine::Property::INT8)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_empty_token.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an empty token");
}

TEST(ASCII, ListSizeTooLarge) {
  auto impl = [](const std::string& name) {
    std::unordered_map<
        std::string_view,
        std::pair<uint64_t, std::unordered_map<
                                std::string_view,
                                std::pair<size_t, plyodine::Property::Type>>>>
        properties = {
            {"vertex",
             {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                              testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
        std::pair<uint64_t, std::unordered_map<
                                std::string_view,
                                std::pair<size_t, plyodine::Property::Type>>>>
        properties = {
            {"vertex",
             {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                              testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
        std::pair<uint64_t, std::unordered_map<
                                std::string_view,
                                std::pair<size_t, plyodine::Property::Type>>>>
        properties = {{"vertex", {1u, {{"l", std::make_pair(0, type)}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                              testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
        std::pair<uint64_t, std::unordered_map<
                                std::string_view,
                                std::pair<size_t, plyodine::Property::Type>>>>
        properties = {{"vertex", {1u, {{"l", std::make_pair(0, type)}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                              testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {2u, {{"a", std::make_pair(0, plyodine::Property::INT8)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 1))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_ascii_unused_tokens.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with unused tokens");
}

TEST(ASCII, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {3u,
            {{"a", std::make_pair(0, plyodine::Property::INT8)},
             {"b", std::make_pair(1, plyodine::Property::UINT8)},
             {"c", std::make_pair(2, plyodine::Property::INT16)},
             {"d", std::make_pair(3, plyodine::Property::UINT16)},
             {"e", std::make_pair(4, plyodine::Property::INT32)},
             {"f", std::make_pair(5, plyodine::Property::UINT32)},
             {"g", std::make_pair(6, plyodine::Property::FLOAT)},
             {"h", std::make_pair(7, plyodine::Property::DOUBLE)}}}},
          {"vertex_lists",
           {1u,
            {{"a", std::make_pair(8, plyodine::Property::INT8_LIST)},
             {"b", std::make_pair(9, plyodine::Property::UINT8_LIST)},
             {"c", std::make_pair(10, plyodine::Property::INT16_LIST)},
             {"d", std::make_pair(11, plyodine::Property::UINT16_LIST)},
             {"e", std::make_pair(12, plyodine::Property::INT32_LIST)},
             {"f", std::make_pair(13, plyodine::Property::UINT32_LIST)},
             {"g", std::make_pair(14, plyodine::Property::FLOAT_LIST)},
             {"h", std::make_pair(15, plyodine::Property::DOUBLE_LIST)}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), ValuesAre(comments),
                            ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", 8, ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", 9, ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", 10, ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", "d", 11,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", 12, ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", "f", 13,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", 14, ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", "h", 15,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

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
    EXPECT_CALL(reader, Start(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_ascii_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithIntListSizes) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_ascii_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithNegativeInt8ListSize) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
  EXPECT_CALL(reader,
              Start(testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_big_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {3u,
            {{"a", std::make_pair(0, plyodine::Property::INT8)},
             {"b", std::make_pair(1, plyodine::Property::UINT8)},
             {"c", std::make_pair(2, plyodine::Property::INT16)},
             {"d", std::make_pair(3, plyodine::Property::UINT16)},
             {"e", std::make_pair(4, plyodine::Property::INT32)},
             {"f", std::make_pair(5, plyodine::Property::UINT32)},
             {"g", std::make_pair(6, plyodine::Property::FLOAT)},
             {"h", std::make_pair(7, plyodine::Property::DOUBLE)}}}},
          {"vertex_lists",
           {1u,
            {{"a", std::make_pair(8, plyodine::Property::INT8_LIST)},
             {"b", std::make_pair(9, plyodine::Property::UINT8_LIST)},
             {"c", std::make_pair(10, plyodine::Property::INT16_LIST)},
             {"d", std::make_pair(11, plyodine::Property::UINT16_LIST)},
             {"e", std::make_pair(12, plyodine::Property::INT32_LIST)},
             {"f", std::make_pair(13, plyodine::Property::UINT32_LIST)},
             {"g", std::make_pair(14, plyodine::Property::FLOAT_LIST)},
             {"h", std::make_pair(15, plyodine::Property::DOUBLE_LIST)}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), ValuesAre(comments),
                            ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", 8, ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", 9, ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", 10, ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", "d", 11,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", 12, ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", "f", 13,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", 14, ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", "h", 15,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream("plyodine/test_data/ply_big_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithDataError) {
  RunReadErrorTest("plyodine/test_data/ply_big_data.ply");
}

TEST(BigEndian, WithUIntListSizes) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
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
    EXPECT_CALL(reader, Start(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
  EXPECT_CALL(reader,
              Start(testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  EXPECT_CALL(reader,
              HandleInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt16List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleUInt32List(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloat(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleFloatList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDouble(testing::_, testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader,
              HandleDoubleList(testing::_, testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream("plyodine/test_data/ply_little_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithData) {
  std::unordered_map<
      std::string_view,
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {3u,
            {{"a", std::make_pair(0, plyodine::Property::INT8)},
             {"b", std::make_pair(1, plyodine::Property::UINT8)},
             {"c", std::make_pair(2, plyodine::Property::INT16)},
             {"d", std::make_pair(3, plyodine::Property::UINT16)},
             {"e", std::make_pair(4, plyodine::Property::INT32)},
             {"f", std::make_pair(5, plyodine::Property::UINT32)},
             {"g", std::make_pair(6, plyodine::Property::FLOAT)},
             {"h", std::make_pair(7, plyodine::Property::DOUBLE)}}}},
          {"vertex_lists",
           {1u,
            {{"a", std::make_pair(8, plyodine::Property::INT8_LIST)},
             {"b", std::make_pair(9, plyodine::Property::UINT8_LIST)},
             {"c", std::make_pair(10, plyodine::Property::INT16_LIST)},
             {"d", std::make_pair(11, plyodine::Property::UINT16_LIST)},
             {"e", std::make_pair(12, plyodine::Property::INT32_LIST)},
             {"f", std::make_pair(13, plyodine::Property::UINT32_LIST)},
             {"g", std::make_pair(14, plyodine::Property::FLOAT_LIST)},
             {"h", std::make_pair(15, plyodine::Property::DOUBLE_LIST)}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), ValuesAre(comments),
                            ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt8("vertex", "a", 0, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", "b", 1, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt16("vertex", "c", 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", "d", 3, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, -1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleInt32("vertex", "e", 4, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 1))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 2))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", "f", 5, 0))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 6, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 7, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt8List("vertex_lists", "a", 8, ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUInt8List("vertex_lists", "b", 9, ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt16List("vertex_lists", "c", 10, ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", "d", 11,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleInt32List("vertex_lists", "e", 12, ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", "f", 13,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", 14, ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", "h", 15,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

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
    EXPECT_CALL(reader, Start(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string_view>()));
    EXPECT_CALL(reader,
                HandleInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader,
                HandleInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader,
                HandleUInt8(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader,
                HandleUInt8List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader,
                HandleInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader,
                HandleInt16List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader,
                HandleUInt16(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader,
                HandleInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader,
                HandleInt32List(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader,
                HandleUInt32(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader,
                HandleFloat(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader,
                HandleFloatList(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader,
                HandleDouble(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u,
            {{"l0", std::make_pair(0, plyodine::Property::UINT8_LIST)},
             {"l1", std::make_pair(1, plyodine::Property::UINT8_LIST)},
             {"l2", std::make_pair(2, plyodine::Property::UINT8_LIST)},
             {"l3", std::make_pair(3, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l0", 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l1", 1, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l2", 2, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUInt8List("vertex", "l3", 3, ValuesAre(values)))
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
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
      std::pair<uint64_t, std::unordered_map<
                              std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>>
      properties = {
          {"vertex",
           {1u, {{"l", std::make_pair(0, plyodine::Property::UINT8_LIST)}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, Start(PropertiesAre(properties), testing::IsEmpty(),
                            testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string_view>()));

  std::ifstream stream(
      "plyodine/test_data/ply_little_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}
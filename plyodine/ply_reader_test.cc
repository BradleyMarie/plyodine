#include "plyodine/ply_reader.h"

#include <fstream>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

enum class PropertyType {
  INT8 = 0u,
  INT8_LIST = 1u,
  UINT8 = 2u,
  UINT8_LIST = 3u,
  INT16 = 4u,
  INT16_LIST = 5u,
  UINT16 = 6u,
  UINT16_LIST = 7u,
  INT32 = 8u,
  INT32_LIST = 9u,
  UINT32 = 10u,
  UINT32_LIST = 11u,
  FLOAT = 12u,
  FLOAT_LIST = 13u,
  DOUBLE = 14u,
  DOUBLE_LIST = 15u
};

class MockPlyReader final : public plyodine::PlyReader {
 public:
  bool initialize_callbacks = true;

  MOCK_METHOD((std::expected<void, std::string>), StartImpl,
              ((const std::map<
                   std::string,
                   std::pair<uintmax_t, std::map<std::string, PropertyType>>>&),
               const std::vector<std::string>&,
               const std::vector<std::string>&),
              ());

  MOCK_METHOD((std::expected<void, std::string>), HandleInt8,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int8_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleInt8List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int8_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleUInt8,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint8_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleUInt8List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint8_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleInt16,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int16_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleInt16List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int16_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleUInt16,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint16_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleUInt16List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint16_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleInt32,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, int32_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleInt32List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const int32_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleUInt32,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, uint32_t));
  MOCK_METHOD((std::expected<void, std::string>), HandleUInt32List,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const uint32_t>));

  MOCK_METHOD((std::expected<void, std::string>), HandleFloat,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, float));
  MOCK_METHOD((std::expected<void, std::string>), HandleFloatList,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const float>));

  MOCK_METHOD((std::expected<void, std::string>), HandleDouble,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, double));
  MOCK_METHOD((std::expected<void, std::string>), HandleDoubleList,
              (const std::string&, size_t, const std::string&, size_t,
               uintmax_t, std::span<const double>));

  std::expected<void, std::string> Start(
      const std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, Callback>>& callbacks,
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

    auto error = StartImpl(properties, comments, object_info);
    if (!error) {
      return std::unexpected(error.error());
    }

    std::map<std::string, std::map<std::string, plyodine::PlyReader::Callback>>
        result;
    if (!initialize_callbacks) {
      return std::expected<void, std::string>();
    }

    for (const auto& element : properties) {
      for (const auto& property : element.second.second) {
        switch (property.second) {
          case PropertyType::INT8:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int8PropertyCallback(
                    &MockPlyReader::HandleInt8);
            break;
          case PropertyType::INT8_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int8PropertyListCallback(
                    &MockPlyReader::HandleInt8List);
            break;
          case PropertyType::UINT8:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt8PropertyCallback(
                    &MockPlyReader::HandleUInt8);
            break;
          case PropertyType::UINT8_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt8PropertyListCallback(
                    &MockPlyReader::HandleUInt8List);
            break;
          case PropertyType::INT16:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int16PropertyCallback(
                    &MockPlyReader::HandleInt16);
            break;
          case PropertyType::INT16_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int16PropertyListCallback(
                    &MockPlyReader::HandleInt16List);
            break;
          case PropertyType::UINT16:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt16PropertyCallback(
                    &MockPlyReader::HandleUInt16);
            break;
          case PropertyType::UINT16_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt16PropertyListCallback(
                    &MockPlyReader::HandleUInt16List);
            break;
          case PropertyType::INT32:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int32PropertyCallback(
                    &MockPlyReader::HandleInt32);
            break;
          case PropertyType::INT32_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::Int32PropertyListCallback(
                    &MockPlyReader::HandleInt32List);
            break;
          case PropertyType::UINT32:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt32PropertyCallback(
                    &MockPlyReader::HandleUInt32);
            break;
          case PropertyType::UINT32_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::UInt32PropertyListCallback(
                    &MockPlyReader::HandleUInt32List);
            break;
          case PropertyType::FLOAT:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::FloatPropertyCallback(
                    &MockPlyReader::HandleFloat);
            break;
          case PropertyType::FLOAT_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::FloatPropertyListCallback(
                    &MockPlyReader::HandleFloatList);
            break;
          case PropertyType::DOUBLE:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::DoublePropertyCallback(
                    &MockPlyReader::HandleDouble);
            break;
          case PropertyType::DOUBLE_LIST:
            callbacks[element.first][property.first] =
                plyodine::PlyReader::DoublePropertyListCallback(
                    &MockPlyReader::HandleDoubleList);
            break;
        }
      }
    }

    return std::expected<void, std::string>();
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
  EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(std::expected<void, std::string>()));

  EXPECT_FALSE(reader.ReadFrom(stream));
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

TEST(Error, BadHeader) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_)).Times(0);
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/header_format_bad.ply");
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
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Failed");
}

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, MismatchedLineEndings) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_mismatched_line_endings.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained mismatched line endings");
}

TEST(ASCII, InvalidCharacter) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_invalid_character.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an invalid character");
}

TEST(ASCII, ListMissingEntries) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_missing_entries.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with too few tokens");
}

TEST(ASCII, MissingElement) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"l", PropertyType::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "l", 0, 0, 1))
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_missing_element.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(), "Unexpected EOF");
}

TEST(ASCII, EmptyToken) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {
          {"vertex",
           {2u, {{"a", PropertyType::INT8}, {"b", PropertyType::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, 1))
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty_token.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an empty token");
}

TEST(ASCII, ListSizeTooLarge) {
  auto impl = [](const std::string& name) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
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
        properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained an unparsable property list size");
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
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained an unparsable property entry");
  };

  impl("_main/plyodine/test_data/ply_ascii_entry_bad_double.ply",
       PropertyType::DOUBLE);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_float.ply",
       PropertyType::FLOAT);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int8.ply",
       PropertyType::INT8);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int16.ply",
       PropertyType::INT16);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_int32.ply",
       PropertyType::INT32);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint8.ply",
       PropertyType::UINT8);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint16.ply",
       PropertyType::UINT16);
  impl("_main/plyodine/test_data/ply_ascii_entry_bad_uint32.ply",
       PropertyType::UINT32);
}

TEST(ASCII, EntryTooBig) {
  auto impl = [](const std::string& name, PropertyType type) {
    std::map<std::string,
             std::pair<uintmax_t, std::map<std::string, PropertyType>>>
        properties = {{"vertex", {1u, {{"l", type}}}}};

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                  testing::IsEmpty()))
        .Times(1)
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .Times(0);

    std::ifstream stream = OpenRunfile(name);
    EXPECT_EQ(reader.ReadFrom(stream).error(),
              "The input contained a property entry that was out of range");
  };

  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_double.ply",
       PropertyType::DOUBLE);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_float.ply",
       PropertyType::FLOAT);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int8.ply",
       PropertyType::INT8);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int16.ply",
       PropertyType::INT16);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_int32.ply",
       PropertyType::INT32);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint8.ply",
       PropertyType::UINT8);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint16.ply",
       PropertyType::UINT16);
  impl("_main/plyodine/test_data/ply_ascii_entry_too_large_uint32.ply",
       PropertyType::UINT32);
}

TEST(ASCII, UnusedTokens) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {2u, {{"a", PropertyType::INT8}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0u, 1))
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_unused_tokens.ply");
  EXPECT_EQ(reader.ReadFrom(stream).error(),
            "The input contained an element with unused tokens");
}

TEST(ASCII, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index,
                          size_t index) -> std::expected<void, std::string> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
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
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(ASCII, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(ASCII, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(ASCII, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithDataError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_data.ply");
}

TEST(BigEndian, WithUIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithUIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_list_sizes.ply", 1000u);
}

TEST(BigEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index,
                          size_t index) -> std::expected<void, std::string> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
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
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(BigEndian, WithIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_list_sizes_signed.ply",
                   1000u);
}

TEST(BigEndian, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(BigEndian, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_big_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(testing::IsEmpty(), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_, testing::_,
                                 testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_empty.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithData) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8("vertex", 0, "a", 0, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt8("vertex", 0, "b", 1, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt16("vertex", 0, "c", 2, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt16("vertex", 0, "d", 3, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 0, -1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt32("vertex", 0, "e", 4, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 0, 1))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 1, 2))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleUInt32("vertex", 0, "f", 5, 2, 0))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 0, 1.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 1, 2.5f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleFloat("vertex", 0, "g", 6, 2, 3.14159274f))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  {
    testing::InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 0, 1.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleDouble("vertex", 0, "h", 7, 1, 2.5))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader,
                HandleDouble("vertex", 0, "h", 7, 2, 3.1415926535897931))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt8List("vertex_lists", 1, "a", 0, 0,
                                     ValuesAre(values_int8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt8List("vertex_lists", 1, "b", 1, 0,
                                      ValuesAre(values_uint8)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt16List("vertex_lists", 1, "c", 2, 0,
                                      ValuesAre(values_int16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt16List("vertex_lists", 1, "d", 3, 0,
                                       ValuesAre(values_uint16)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader, HandleInt32List("vertex_lists", 1, "e", 4, 0,
                                      ValuesAre(values_int32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader, HandleUInt32List("vertex_lists", 1, "f", 5, 0,
                                       ValuesAre(values_uint32)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader, HandleFloatList("vertex_lists", 1, "g", 6, 0,
                                      ValuesAre(values_float)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader, HandleDoubleList("vertex_lists", 1, "h", 7, 0,
                                       ValuesAre(values_double)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithDataSkipAll) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {3u,
                      {{"a", PropertyType::INT8},
                       {"b", PropertyType::UINT8},
                       {"c", PropertyType::INT16},
                       {"d", PropertyType::UINT16},
                       {"e", PropertyType::INT32},
                       {"f", PropertyType::UINT32},
                       {"g", PropertyType::FLOAT},
                       {"h", PropertyType::DOUBLE}}}},
                    {"vertex_lists",
                     {1u,
                      {{"a", PropertyType::INT8_LIST},
                       {"b", PropertyType::UINT8_LIST},
                       {"c", PropertyType::INT16_LIST},
                       {"d", PropertyType::UINT16_LIST},
                       {"e", PropertyType::INT32_LIST},
                       {"f", PropertyType::UINT32_LIST},
                       {"g", PropertyType::FLOAT_LIST},
                       {"h", PropertyType::DOUBLE_LIST}}}}};
  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  MockPlyReader reader;
  reader.initialize_callbacks = false;

  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), ValuesAre(comments),
                                ValuesAre(object_info)))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));
  EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                  testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                      testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
      .Times(0);
  EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
      .Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithDataError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_data.ply");
}

TEST(LittleEndian, HandleFails) {
  auto impl = [](size_t index) {
    auto make_result = [](size_t case_index,
                          size_t index) -> std::expected<void, std::string> {
      if (case_index == index) {
        return std::unexpected("Failed");
      }

      return std::expected<void, std::string>();
    };

    MockPlyReader reader;
    EXPECT_CALL(reader, StartImpl(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(std::expected<void, std::string>()));
    EXPECT_CALL(reader, HandleInt8(testing::_, testing::_, testing::_,
                                   testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleInt8List(testing::_, testing::_, testing::_,
                                       testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUInt8(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUInt8List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleInt16(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleInt16List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUInt16(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUInt16List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt32(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleInt32List(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt32(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUInt32List(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(testing::_, testing::_, testing::_,
                                    testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(testing::_, testing::_, testing::_,
                                        testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(testing::_, testing::_, testing::_,
                                     testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(testing::_, testing::_, testing::_,
                                         testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(make_result(15u, index)));

    std::ifstream stream =
        OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
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
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<uint8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithUIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_list_sizes.ply",
                   1000u);
}

TEST(LittleEndian, WithIntListSizes) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex",
                     {1u,
                      {{"l0", PropertyType::UINT8_LIST},
                       {"l1", PropertyType::UINT8_LIST},
                       {"l2", PropertyType::UINT8_LIST},
                       {"l3", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::vector<uint8_t> values;

  values.resize(std::numeric_limits<int8_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l0", 0, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l1", 1, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l2", 2, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader,
              HandleUInt8List("vertex", 0, "l3", 3, 0, ValuesAre(values)))
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_signed.ply");
  EXPECT_TRUE(reader.ReadFrom(stream));
}

TEST(LittleEndian, WithIntListSizesError) {
  RunReadErrorTest(
      "_main/plyodine/test_data/ply_little_signed_list_sizes.ply", 1000u);
}

TEST(LittleEndian, WithNegativeInt8ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int8.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, WithNegativeInt16ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int16.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}

TEST(LittleEndian, WithNegativeInt32ListSize) {
  std::map<std::string,
           std::pair<uintmax_t, std::map<std::string, PropertyType>>>
      properties = {{"vertex", {1u, {{"l", PropertyType::UINT8_LIST}}}}};

  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(PropertiesAre(properties), testing::IsEmpty(),
                                testing::IsEmpty()))
      .Times(1)
      .WillOnce(testing::Return(std::expected<void, std::string>()));

  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_little_list_sizes_negative_int32.ply");
  auto result = reader.ReadFrom(stream);
  EXPECT_EQ("The input contained a property list with a negative size",
            result.error());
}
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
           std::unordered_map<std::string_view,
                              std::pair<size_t, plyodine::Property::Type>>>&),
       std::span<const std::string_view>),
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

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, Start(testing::IsEmpty(), testing::IsEmpty()))
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

TEST(BigEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, Start(testing::IsEmpty(), testing::IsEmpty()))
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

TEST(LittleEndian, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, Start(testing::IsEmpty(), testing::IsEmpty()))
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
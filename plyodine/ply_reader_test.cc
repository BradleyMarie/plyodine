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
  bool add_invalid_element = false;
  bool add_invalid_property = false;
  bool convert_float_to_int = false;
  bool convert_int_to_float = false;
  bool convert_list_to_scalar = false;
  bool convert_scalar_to_list = false;

  MOCK_METHOD(std::error_code, StartImpl,
              ((const std::map<
                   std::string,
                   std::pair<uintmax_t, std::map<std::string, PropertyType>>>&),
               const std::vector<std::string>&,
               const std::vector<std::string>&),
              ());

  MOCK_METHOD(std::error_code, HandleChar,
              (const std::string&, const std::string&, int8_t));
  MOCK_METHOD(std::error_code, HandleCharList,
              (const std::string&, const std::string&,
               std::span<const int8_t>));

  MOCK_METHOD(std::error_code, HandleUChar,
              (const std::string&, const std::string&, uint8_t));
  MOCK_METHOD(std::error_code, HandleUCharList,
              (const std::string&, const std::string&,
               std::span<const uint8_t>));

  MOCK_METHOD(std::error_code, HandleShort,
              (const std::string&, const std::string&, int16_t));
  MOCK_METHOD(std::error_code, HandleShortList,
              (const std::string&, const std::string&,
               std::span<const int16_t>));

  MOCK_METHOD(std::error_code, HandleUShort,
              (const std::string&, const std::string&, uint16_t));
  MOCK_METHOD(std::error_code, HandleUShortList,
              (const std::string&, const std::string&,
               std::span<const uint16_t>));

  MOCK_METHOD(std::error_code, HandleInt,
              (const std::string&, const std::string&, int32_t));
  MOCK_METHOD(std::error_code, HandleIntList,
              (const std::string&, const std::string&,
               std::span<const int32_t>));

  MOCK_METHOD(std::error_code, HandleUInt,
              (const std::string&, const std::string&, uint32_t));
  MOCK_METHOD(std::error_code, HandleUIntList,
              (const std::string&, const std::string&,
               std::span<const uint32_t>));

  MOCK_METHOD(std::error_code, HandleFloat,
              (const std::string&, const std::string&, float));
  MOCK_METHOD(std::error_code, HandleFloatList,
              (const std::string&, const std::string&, std::span<const float>));

  MOCK_METHOD(std::error_code, HandleDouble,
              (const std::string&, const std::string&, double));
  MOCK_METHOD(std::error_code, HandleDoubleList,
              (const std::string&, const std::string&,
               std::span<const double>));

  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) override {
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

    if (add_invalid_element) {
      callbacks.try_emplace("invalid_element");
    }

    for (const auto& element : properties) {
      if (add_invalid_property) {
        callbacks[element.first].try_emplace("invalid_property");
      }

      for (const auto& property : element.second.second) {
        switch (property.second) {
          case PropertyType::CHAR:
            callbacks[element.first][property.first] = CharPropertyCallback(
                [element_name = element.first, property_name = property.first,
                 this](int8_t value) {
                  return HandleChar(element_name, property_name, value);
                });
            break;
          case PropertyType::CHAR_LIST:
            callbacks[element.first][property.first] = CharPropertyListCallback(
                [element_name = element.first, property_name = property.first,
                 this](std::span<const int8_t> value) {
                  return HandleCharList(element_name, property_name, value);
                });
            break;
          case PropertyType::UCHAR:
            callbacks[element.first][property.first] = UCharPropertyCallback(
                [element_name = element.first, property_name = property.first,
                 this](uint8_t value) {
                  return HandleUChar(element_name, property_name, value);
                });
            break;
          case PropertyType::UCHAR_LIST:
            callbacks[element.first][property.first] =
                UCharPropertyListCallback([element_name = element.first,
                                           property_name = property.first,
                                           this](
                                              std::span<const uint8_t> value) {
                  return HandleUCharList(element_name, property_name, value);
                });
            break;
          case PropertyType::SHORT:
            callbacks[element.first][property.first] = ShortPropertyCallback(
                [element_name = element.first, property_name = property.first,
                 this](int16_t value) {
                  return HandleShort(element_name, property_name, value);
                });
            break;
          case PropertyType::SHORT_LIST:
            callbacks[element.first][property.first] =
                ShortPropertyListCallback([element_name = element.first,
                                           property_name = property.first,
                                           this](
                                              std::span<const int16_t> value) {
                  return HandleShortList(element_name, property_name, value);
                });
            break;
          case PropertyType::USHORT:
            callbacks[element.first][property.first] = UShortPropertyCallback(
                [element_name = element.first, property_name = property.first,
                 this](uint16_t value) {
                  return HandleUShort(element_name, property_name, value);
                });
            break;
          case PropertyType::USHORT_LIST:
            callbacks[element.first][property.first] =
                UShortPropertyListCallback(
                    [element_name = element.first,
                     property_name = property.first,
                     this](std::span<const uint16_t> value) {
                      return HandleUShortList(element_name, property_name,
                                              value);
                    });
            break;
          case PropertyType::INT:
            callbacks[element.first][property.first] = IntPropertyCallback(
                [element_name = element.first, property_name = property.first,
                 this](int32_t value) {
                  return HandleInt(element_name, property_name, value);
                });
            break;
          case PropertyType::INT_LIST:
            callbacks[element.first][property.first] = IntPropertyListCallback(
                [element_name = element.first, property_name = property.first,
                 this](std::span<const int32_t> value) {
                  return HandleIntList(element_name, property_name, value);
                });
            break;
          case PropertyType::UINT:
            if (convert_int_to_float) {
              callbacks[element.first][property.first] =
                  FloatPropertyListCallback([element_name = element.first,
                                             property_name = property.first,
                                             this](
                                                std::span<const float> value) {
                    return HandleFloatList(element_name, property_name, value);
                  });
            } else {
              callbacks[element.first][property.first] = UIntPropertyCallback(
                  [element_name = element.first, property_name = property.first,
                   this](uint32_t value) {
                    return HandleUInt(element_name, property_name, value);
                  });
            }
            break;
          case PropertyType::UINT_LIST:
            callbacks[element.first][property.first] = UIntPropertyListCallback(
                [element_name = element.first, property_name = property.first,
                 this](std::span<const uint32_t> value) {
                  return HandleUIntList(element_name, property_name, value);
                });
            break;
          case PropertyType::FLOAT:
            if (convert_float_to_int) {
              callbacks[element.first][property.first] = UIntPropertyCallback(
                  [element_name = element.first, property_name = property.first,
                   this](uint32_t value) {
                    return HandleUInt(element_name, property_name, value);
                  });
            } else {
              callbacks[element.first][property.first] = FloatPropertyCallback(
                  [element_name = element.first, property_name = property.first,
                   this](float value) {
                    return HandleFloat(element_name, property_name, value);
                  });
            }
            break;
          case PropertyType::FLOAT_LIST:
            callbacks[element.first][property.first] =
                FloatPropertyListCallback([element_name = element.first,
                                           property_name = property.first,
                                           this](std::span<const float> value) {
                  return HandleFloatList(element_name, property_name, value);
                });
            break;
          case PropertyType::DOUBLE:
            if (convert_scalar_to_list) {
              callbacks[element.first][property.first] =
                  DoublePropertyListCallback(
                      [element_name = element.first,
                       property_name = property.first,
                       this](std::span<const double> value) {
                        return HandleDoubleList(element_name, property_name,
                                                value);
                      });
            } else {
              callbacks[element.first][property.first] = DoublePropertyCallback(
                  [element_name = element.first, property_name = property.first,
                   this](double value) {
                    return HandleDouble(element_name, property_name, value);
                  });
            }
            break;
          case PropertyType::DOUBLE_LIST:
            if (convert_list_to_scalar) {
              callbacks[element.first][property.first] = DoublePropertyCallback(
                  [element_name = element.first, property_name = property.first,
                   this](double value) {
                    return HandleDouble(element_name, property_name, value);
                  });
            } else {
              callbacks[element.first][property.first] =
                  DoublePropertyListCallback(
                      [element_name = element.first,
                       property_name = property.first,
                       this](std::span<const double> value) {
                        return HandleDoubleList(element_name, property_name,
                                                value);
                      });
            }
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
  EXPECT_CALL(reader, HandleChar(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleCharList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUChar(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUCharList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleShort(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleShortList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUShort(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUShortList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleInt(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleIntList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUInt(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleUIntList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleFloat(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleFloatList(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleDouble(_, _, _))
      .WillRepeatedly(Return(std::error_code()));
  EXPECT_CALL(reader, HandleDoubleList(_, _, _))
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
  for (int i = 1; i <= 20; i++) {
    EXPECT_EQ(error_catgegory.default_error_condition(i),
              std::errc::invalid_argument);
  }
  EXPECT_NE(error_catgegory.default_error_condition(21),
            std::errc::invalid_argument);
}

TEST(Validate, BadStream) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

  std::stringstream input(std::ios::in | std::ios::binary);
  input.clear(std::ios::badbit);

  EXPECT_EQ("Input stream must be in good state",
            reader.ReadFrom(input).message());
}

TEST(Error, BadHeader) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  EXPECT_EQ(reader.ReadFrom(stream).value(), 1);
}

TEST(Error, UnknownElement) {
  MockPlyReader reader;
  reader.add_invalid_element = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map contained an unknown element",
            reader.ReadFrom(stream).message());
}

TEST(Error, UnknownProperty) {
  MockPlyReader reader;
  reader.add_invalid_property = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map contained an unknown property",
            reader.ReadFrom(stream).message());
}

TEST(Error, IntToFloat) {
  MockPlyReader reader;
  reader.convert_int_to_float = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map requested an unsupported conversion",
            reader.ReadFrom(stream).message());
}

TEST(Error, FloatToInt) {
  MockPlyReader reader;
  reader.convert_float_to_int = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map requested an unsupported conversion",
            reader.ReadFrom(stream).message());
}

TEST(Error, ListToScalar) {
  MockPlyReader reader;
  reader.convert_list_to_scalar = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map requested an unsupported conversion",
            reader.ReadFrom(stream).message());
}

TEST(Error, ScalarToList) {
  MockPlyReader reader;
  reader.convert_scalar_to_list = true;

  EXPECT_CALL(reader, StartImpl(_, _, _))
      .Times(1)
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  EXPECT_EQ("The callback map requested an unsupported conversion",
            reader.ReadFrom(stream).message());
}

TEST(ASCII, Empty) {
  MockPlyReader reader;
  EXPECT_CALL(reader, StartImpl(IsEmpty(), IsEmpty(), IsEmpty()))
      .Times(1)
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar("vertex", "l", 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar("vertex", "a", 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
    EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleChar("vertex", "a", 1))
      .WillOnce(Return(std::error_code()));
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar("vertex", "a", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleShort("vertex", "c", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt("vertex", "e", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleCharList("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUCharList("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleShortList("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUShortList("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleIntList("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUIntList("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
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
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleCharList(_, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUChar(_, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUCharList(_, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleShort(_, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleShortList(_, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUShort(_, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUShortList(_, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt(_, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleIntList(_, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt(_, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUIntList(_, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _))
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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(ASCII, WithNegativeCharListSize) {
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

TEST(ASCII, WithNegativeShortListSize) {
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

TEST(ASCII, WithNegativeIntListSize) {
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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar("vertex", "a", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleShort("vertex", "c", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt("vertex", "e", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleCharList("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUCharList("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleShortList("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUShortList("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleIntList("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUIntList("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
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
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
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
    EXPECT_CALL(reader, HandleChar(_, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleCharList(_, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUChar(_, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUCharList(_, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleShort(_, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleShortList(_, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUShort(_, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUShortList(_, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt(_, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleIntList(_, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt(_, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUIntList(_, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _))
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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(BigEndian, WithIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_big_list_sizes_signed.ply",
                   1000u);
}

TEST(BigEndian, WithNegativeCharListSize) {
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

TEST(BigEndian, WithNegativeShortListSize) {
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

TEST(BigEndian, WithNegativeIntListSize) {
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
  EXPECT_CALL(reader, HandleChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar("vertex", "a", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleChar("vertex", "a", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUChar("vertex", "b", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleShort("vertex", "c", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleShort("vertex", "c", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUShort("vertex", "d", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleInt("vertex", "e", -1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleInt("vertex", "e", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 1))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 2))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleUInt("vertex", "f", 0))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 1.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 2.5f))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleFloat("vertex", "g", 3.14159274f))
        .WillOnce(Return(std::error_code()));
  }

  {
    InSequence s;
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 1.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 2.5))
        .WillOnce(Return(std::error_code()));
    EXPECT_CALL(reader, HandleDouble("vertex", "h", 3.1415926535897931))
        .WillOnce(Return(std::error_code()));
  }

  std::vector<int8_t> values_int8 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleCharList("vertex_lists", "a", ValuesAre(values_int8)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint8_t> values_uint8 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUCharList("vertex_lists", "b", ValuesAre(values_uint8)))
      .WillOnce(Return(std::error_code()));

  std::vector<int16_t> values_int16 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleShortList("vertex_lists", "c", ValuesAre(values_int16)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint16_t> values_uint16 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUShortList("vertex_lists", "d", ValuesAre(values_uint16)))
      .WillOnce(Return(std::error_code()));

  std::vector<int32_t> values_int32 = {-1, 2, 0};
  EXPECT_CALL(reader,
              HandleIntList("vertex_lists", "e", ValuesAre(values_int32)))
      .WillOnce(Return(std::error_code()));

  std::vector<uint32_t> values_uint32 = {1, 2, 0};
  EXPECT_CALL(reader,
              HandleUIntList("vertex_lists", "f", ValuesAre(values_uint32)))
      .WillOnce(Return(std::error_code()));

  std::vector<float> values_float = {1.5f, 2.5f, 3.14159274f};
  EXPECT_CALL(reader,
              HandleFloatList("vertex_lists", "g", ValuesAre(values_float)))
      .WillOnce(Return(std::error_code()));

  std::vector<double> values_double = {1.5, 2.5, 3.1415926535897931};
  EXPECT_CALL(reader,
              HandleDoubleList("vertex_lists", "h", ValuesAre(values_double)))
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
  EXPECT_CALL(reader, HandleCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUChar(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUCharList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShort(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUShortList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUInt(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleUIntList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloat(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleFloatList(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDouble(_, _, _)).Times(0);
  EXPECT_CALL(reader, HandleDoubleList(_, _, _)).Times(0);

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
    EXPECT_CALL(reader, HandleChar(_, _, _))
        .WillRepeatedly(Return(make_result(0u, index)));
    EXPECT_CALL(reader, HandleCharList(_, _, _))
        .WillRepeatedly(Return(make_result(1u, index)));
    EXPECT_CALL(reader, HandleUChar(_, _, _))
        .WillRepeatedly(Return(make_result(2u, index)));
    EXPECT_CALL(reader, HandleUCharList(_, _, _))
        .WillRepeatedly(Return(make_result(3u, index)));
    EXPECT_CALL(reader, HandleShort(_, _, _))
        .WillRepeatedly(Return(make_result(4u, index)));
    EXPECT_CALL(reader, HandleShortList(_, _, _))
        .WillRepeatedly(Return(make_result(5u, index)));
    EXPECT_CALL(reader, HandleUShort(_, _, _))
        .WillRepeatedly(Return(make_result(6u, index)));
    EXPECT_CALL(reader, HandleUShortList(_, _, _))
        .WillRepeatedly(Return(make_result(7u, index)));
    EXPECT_CALL(reader, HandleInt(_, _, _))
        .WillRepeatedly(Return(make_result(8u, index)));
    EXPECT_CALL(reader, HandleIntList(_, _, _))
        .WillRepeatedly(Return(make_result(9u, index)));
    EXPECT_CALL(reader, HandleUInt(_, _, _))
        .WillRepeatedly(Return(make_result(10u, index)));
    EXPECT_CALL(reader, HandleUIntList(_, _, _))
        .WillRepeatedly(Return(make_result(11u, index)));
    EXPECT_CALL(reader, HandleFloat(_, _, _))
        .WillRepeatedly(Return(make_result(12u, index)));
    EXPECT_CALL(reader, HandleFloatList(_, _, _))
        .WillRepeatedly(Return(make_result(13u, index)));
    EXPECT_CALL(reader, HandleDouble(_, _, _))
        .WillRepeatedly(Return(make_result(14u, index)));
    EXPECT_CALL(reader, HandleDoubleList(_, _, _))
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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint8_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<uint16_t>::max() + 1u, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
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
  EXPECT_CALL(reader, HandleUCharList("vertex", "l0", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int8_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l1", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max(), 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l2", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  values.resize(std::numeric_limits<int16_t>::max() + 1, 136u);
  EXPECT_CALL(reader, HandleUCharList("vertex", "l3", ValuesAre(values)))
      .WillOnce(Return(std::error_code()));

  std::ifstream stream =
      OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes_signed.ply");
  EXPECT_EQ(0, reader.ReadFrom(stream).value());
}

TEST(LittleEndian, WithIntListSizesError) {
  RunReadErrorTest("_main/plyodine/test_data/ply_little_signed_list_sizes.ply",
                   1000u);
}

TEST(LittleEndian, WithNegativeCharListSize) {
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

TEST(LittleEndian, WithNegativeShortListSize) {
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

TEST(LittleEndian, WithNegativeIntListSize) {
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

class MockConvertingPlyReader final : public PlyReader {
 public:
  MockConvertingPlyReader(PropertyType type) : type_(type) {}

  MOCK_METHOD(std::error_code, OnConversionFailure,
              (const std::string&, const std::string&, int));

 private:
  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) override {
    static const PropertyCallback property_callbacks[16] = {
        PropertyCallback(std::in_place_index<0>),
        PropertyCallback(std::in_place_index<1>),
        PropertyCallback(std::in_place_index<2>),
        PropertyCallback(std::in_place_index<3>),
        PropertyCallback(std::in_place_index<4>),
        PropertyCallback(std::in_place_index<5>),
        PropertyCallback(std::in_place_index<6>),
        PropertyCallback(std::in_place_index<7>),
        PropertyCallback(std::in_place_index<8>),
        PropertyCallback(std::in_place_index<9>),
        PropertyCallback(std::in_place_index<10>),
        PropertyCallback(std::in_place_index<11>),
        PropertyCallback(std::in_place_index<12>),
        PropertyCallback(std::in_place_index<13>),
        PropertyCallback(std::in_place_index<14>),
        PropertyCallback(std::in_place_index<15>)};
    callbacks["vertex"]["a"] = property_callbacks[static_cast<size_t>(type_)];
    return std::error_code();
  }

  std::error_code OnConversionFailure(const std::string& element_name,
                                      const std::string& property_name,
                                      ConversionFailureReason reason) override {
    return OnConversionFailure(element_name, property_name,
                               static_cast<int>(reason));
  }

  PropertyType type_;
};

TEST(Error, OverridesError) {
  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_limit_negative_int32.ply");

  MockConvertingPlyReader reader(PropertyType::UCHAR);
  EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 0))
      .WillOnce(Return(std::error_code(1, std::generic_category())));

  EXPECT_EQ(1, reader.ReadFrom(stream).value());
}

TEST(Error, NegativeToUnsigned) {
  PropertyType types[]{PropertyType::UCHAR, PropertyType::USHORT,
                       PropertyType::UINT};
  for (PropertyType type : types) {
    std::ifstream stream = OpenRunfile(
        "_main/plyodine/test_data/ply_ascii_limit_negative_int32.ply");

    MockConvertingPlyReader reader(type);
    EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 0))
        .WillOnce(Return(std::error_code()));

    EXPECT_EQ("Signed integer to unsigned integer conversion underflowed",
              reader.ReadFrom(stream).message());
  }
}

TEST(Error, NegativeSignedUnderflow) {
  PropertyType types[]{PropertyType::CHAR, PropertyType::SHORT};
  for (PropertyType type : types) {
    std::ifstream stream = OpenRunfile(
        "_main/plyodine/test_data/ply_ascii_limit_negative_int32.ply");

    MockConvertingPlyReader reader(type);
    EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 1))
        .WillOnce(Return(std::error_code()));

    EXPECT_EQ("Signed integer conversion underflowed",
              reader.ReadFrom(stream).message());
  }
}

TEST(Error, IntegerOverflow) {
  PropertyType types[]{PropertyType::CHAR, PropertyType::UCHAR,
                       PropertyType::SHORT, PropertyType::USHORT,
                       PropertyType::INT};
  for (PropertyType type : types) {
    std::ifstream stream = OpenRunfile(
        "_main/plyodine/test_data/ply_ascii_limit_positive_uint32.ply");

    MockConvertingPlyReader reader(type);
    EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 2))
        .WillOnce(Return(std::error_code()));

    EXPECT_EQ("Integer conversion overflowed",
              reader.ReadFrom(stream).message());
  }
}

TEST(Error, FloatUnderflow) {
  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_limit_negative_double.ply");

  MockConvertingPlyReader reader(PropertyType::FLOAT);
  EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 3))
      .WillOnce(Return(std::error_code()));

  EXPECT_EQ("Floating point conversion underflowed",
            reader.ReadFrom(stream).message());
}

TEST(Error, FloatOverflow) {
  std::ifstream stream = OpenRunfile(
      "_main/plyodine/test_data/ply_ascii_limit_positive_double.ply");

  MockConvertingPlyReader reader(PropertyType::FLOAT);
  EXPECT_CALL(reader, OnConversionFailure("vertex", "a", 4))
      .WillOnce(Return(std::error_code()));

  EXPECT_EQ("Floating point conversion overflowed",
            reader.ReadFrom(stream).message());
}

}  // namespace
}  // namespace plyodine
#include "plyodine/writers/in_memory_writer.h"

#include <bit>
#include <limits>
#include <sstream>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

TEST(List, UInt8) {
  static const std::vector<uint8_t> values(std::numeric_limits<uint8_t>::max(),
                                           std::numeric_limits<uint8_t>::max());
  static const std::vector<std::span<const uint8_t>> l0 = {
      {values.begin(), values.end()}};

  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      result;
  result["vertex"]["l0"] = l0;

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, result));

  EXPECT_THAT(
      output.str(),
      testing::StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                          "list uchar uchar l0\rend_header\r"));
}

TEST(List, UInt16) {
  static const std::vector<uint8_t> values(std::numeric_limits<uint16_t>::max(),
                                           std::numeric_limits<uint8_t>::max());
  static const std::vector<std::span<const uint8_t>> l0 = {
      {values.begin(), values.end()}};

  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      result;
  result["vertex"]["l0"] = l0;

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, result));

  EXPECT_THAT(
      output.str(),
      testing::StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                          "list ushort uchar l0\rend_header\r"));
}

TEST(List, UInt32) {
  static const std::vector<uint8_t> values(
      std::numeric_limits<uint16_t>::max() + 1u,
      std::numeric_limits<uint8_t>::max());
  static const std::vector<std::span<const uint8_t>> l0 = {
      {values.begin(), values.end()}};

  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      result;
  result["vertex"]["l0"] = l0;

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, result));

  EXPECT_THAT(
      output.str(),
      testing::StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                          "list uint uchar l0\rend_header\r"));
}

TEST(List, TooLargeError) {
  if constexpr (std::numeric_limits<size_t>::max() >
                std::numeric_limits<uint32_t>::max()) {
    static const std::vector<uint8_t> values(
        std::numeric_limits<uint32_t>::max() + 1ull,
        std::numeric_limits<uint8_t>::max());
    static const std::vector<std::span<const uint8_t>> l0 = {
        {values.begin(), values.end()}};

    std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
        result;
    result["vertex"]["l0"] = l0;

    std::stringstream output;
    EXPECT_EQ(plyodine::WriteToASCII(output, result).error(),
              "Property lists can contain no more than 4294967295 entries");
  }
}

TEST(Properties, DifferentSizes) {
  static const std::vector<int> l0 = {1, 2};
  static const std::vector<int> l1 = {1, 2, 3};

  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      result;
  result["vertex"]["l0"] = l0;
  result["vertex"]["l1"] = l1;

  std::stringstream output;
  EXPECT_EQ(plyodine::WriteToASCII(output, result).error(),
            "All properties of an element must have the same size");
}

TEST(ASCII, WithData) {
  static const std::vector<int8_t> a = {-1, 2, 0};
  static const std::vector<uint8_t> b = {1u, 2u, 0u};
  static const std::vector<int16_t> c = {-1, 2, 0};
  static const std::vector<uint16_t> d = {1u, 2u, 0u};
  static const std::vector<int32_t> e = {-1, 2, 0};
  static const std::vector<uint32_t> f = {1u, 2u, 0u};
  static const std::vector<float> g = {1.5, 2.5, std::acos(-1.0f)};
  static const std::vector<double> h = {1.5, 2.5, std::acos(-1.0)};
  static const std::vector<std::span<const int8_t>> al = {{a}};
  static const std::vector<std::span<const uint8_t>> bl = {{b}};
  static const std::vector<std::span<const int16_t>> cl = {{c}};
  static const std::vector<std::span<const uint16_t>> dl = {{d}};
  static const std::vector<std::span<const int32_t>> el = {{e}};
  static const std::vector<std::span<const uint32_t>> fl = {{f}};
  static const std::vector<std::span<const float>> gl = {{g}};
  static const std::vector<std::span<const double>> hl = {{h}};

  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      result;
  result["vertex"]["a"] = a;
  result["vertex"]["b"] = b;
  result["vertex"]["c"] = c;
  result["vertex"]["d"] = d;
  result["vertex"]["e"] = e;
  result["vertex"]["f"] = f;
  result["vertex"]["g"] = g;
  result["vertex"]["h"] = h;
  result["vertex_lists"]["a"] = al;
  result["vertex_lists"]["b"] = bl;
  result["vertex_lists"]["c"] = cl;
  result["vertex_lists"]["d"] = dl;
  result["vertex_lists"]["e"] = el;
  result["vertex_lists"]["f"] = fl;
  result["vertex_lists"]["g"] = gl;
  result["vertex_lists"]["h"] = hl;

  std::vector<std::string> comments = {"comment 1", "comment 2"};
  std::vector<std::string> object_info = {"obj info 1", "obj info 2"};

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, result, comments, object_info));

  std::stringstream input(
      "ply\r"
      "format ascii 1.0\r"
      "comment comment 1\r"
      "comment comment 2\r"
      "obj_info obj info 1\r"
      "obj_info obj info 2\r"
      "element vertex 3\r"
      "property char a\r"
      "property uchar b\r"
      "property short c\r"
      "property ushort d\r"
      "property int e\r"
      "property uint f\r"
      "property float g\r"
      "property double h\r"
      "element vertex_lists 1\r"
      "property list uchar char a\r"
      "property list uchar uchar b\r"
      "property list uchar short c\r"
      "property list uchar ushort d\r"
      "property list uchar int e\r"
      "property list uchar uint f\r"
      "property list uchar float g\r"
      "property list uchar double h\r"
      "end_header\r"
      "-1 1 -1 1 -1 1 1.5 1.5\r"
      "2 2 2 2 2 2 2.5 2.5\r"
      "0 0 0 0 0 0 3.14159274 3.1415926535897931\r"
      "3 -1 2 0 3 1 2 0 3 -1 2 0 3 1 2 0 3 -1 2 0 3 1 2 0 3 1.5 2.5 3.14159274 "
      "3 1.5 2.5 3.1415926535897931\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, {}));

  std::stringstream input("ply\rformat ascii 1.0\rend_header\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToBigEndian(output, {}));

  std::stringstream input("ply\rformat binary_big_endian 1.0\rend_header\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToLittleEndian(output, {}));

  std::stringstream input("ply\rformat binary_little_endian 1.0\rend_header\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(Native, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteTo(output, {}));

  if constexpr (std::endian::native == std::endian::big) {
    std::stringstream input("ply\rformat binary_big_endian 1.0\rend_header\r");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::stringstream input(
        "ply\rformat binary_little_endian 1.0\rend_header\r");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}
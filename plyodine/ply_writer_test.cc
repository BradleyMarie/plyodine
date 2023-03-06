#include "plyodine/ply_writer.h"

#include <bit>
#include <cmath>
#include <fstream>
#include <iterator>
#include <sstream>

#include "googletest/include/gtest/gtest.h"

std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
BuildTestData() {
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
  return result;
}

TEST(Validate, BadElementNames) {
  std::stringstream output;
  EXPECT_EQ(plyodine::WriteToASCII(output, {{"", {}}}).error(),
            "Names of properties and elements may not be empty");
  EXPECT_EQ(
      plyodine::WriteToASCII(output, {{" ", {}}}).error(),
      "Names of properties and elements may only contain graphic characters");
}

TEST(Validate, BadPropertyNames) {
  std::stringstream output;
  EXPECT_EQ(plyodine::WriteToASCII(output, {{"element", {{"", {}}}}}).error(),
            "Names of properties and elements may not be empty");
  EXPECT_EQ(
      plyodine::WriteToASCII(output, {{"element", {{" ", {}}}}}).error(),
      "Names of properties and elements may only contain graphic characters");
}

TEST(Validate, MismatchedProperties) {
  std::vector<float> elements0;
  std::vector<float> elements1 = {1.0};
  std::stringstream output;
  EXPECT_EQ(plyodine::WriteToASCII(
                output,
                {{"element", {{"node0", {elements0}}, {"node1", {elements1}}}}})
                .error(),
            "All properties of an element must have the same size");
}

TEST(Validate, BadComment) {
  std::stringstream output;
  EXPECT_EQ(plyodine::WriteToASCII(output, {}, {{"\r"}}).error(),
            "A comment may not contain line feed or carriage return");
  EXPECT_EQ(plyodine::WriteToASCII(output, {}, {{"\n"}}).error(),
            "A comment may not contain line feed or carriage return");
}

TEST(Validate, ListTooBig) {
  if constexpr (std::numeric_limits<uint32_t>::max() <
                std::numeric_limits<size_t>::max()) {
    float value;
    std::span<const float> entries(
        &value, static_cast<size_t>(std::numeric_limits<uint32_t>::max()) +
                    static_cast<size_t>(1u));
    std::vector<std::span<const float>> list({entries});

    std::stringstream output;
    EXPECT_EQ(plyodine::WriteToASCII(output, {{"element", {{"node0", {list}}}}})
                  .error(),
              "A property list contained too many values");
  }
}

TEST(ASCII, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, {}));

  std::ifstream input("plyodine/test_data/ply_ascii_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, NonFinite) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      data;
  data["vertex"]["a"] = a;

  std::stringstream output;
  EXPECT_EQ(
      plyodine::WriteToASCII(output, data).error(),
      "Only finite floating point values may be serialized to an ASCII output");
}

TEST(ASCII, NonFiniteList) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::vector<std::span<const float>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  EXPECT_EQ(
      plyodine::WriteToASCII(output, data).error(),
      "Only finite floating point values may be serialized to an ASCII output");
}

TEST(ASCII, TestData) {
  std::string_view comments[] = {{"comment 1"}, {"comment 2"}};
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, BuildTestData(), comments));

  std::ifstream input("plyodine/test_data/ply_ascii_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, LargeFP) {
  std::vector<double> a = {18446744073709551616.0};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, data));

  std::ifstream input("plyodine/test_data/ply_ascii_large_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, SmallFP) {
  std::vector<double> a = {0.000000000000000000000025};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, plyodine::Property>>
      data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToASCII(output, data));

  std::ifstream input("plyodine/test_data/ply_ascii_small_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToBigEndian(output, {}));

  std::ifstream input("plyodine/test_data/ply_big_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, TestData) {
  std::string_view comments[] = {{"comment 1"}, {"comment 2"}};
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToBigEndian(output, BuildTestData(), comments));

  std::ifstream input("plyodine/test_data/ply_big_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToLittleEndian(output, {}));

  std::ifstream input("plyodine/test_data/ply_little_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, TestData) {
  std::string_view comments[] = {{"comment 1"}, {"comment 2"}};
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToLittleEndian(output, BuildTestData(), comments));

  std::ifstream input("plyodine/test_data/ply_little_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(Native, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToBinary(output, {}));

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input("plyodine/test_data/ply_big_empty.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input("plyodine/test_data/ply_little_empty.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}

TEST(Native, TestData) {
  std::stringstream output;
  std::string_view comments[] = {{"comment 1"}, {"comment 2"}};
  ASSERT_TRUE(plyodine::WriteToBinary(output, BuildTestData(), comments));

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input("plyodine/test_data/ply_big_data.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input("plyodine/test_data/ply_little_data.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}

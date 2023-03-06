#include "plyodine/ply_writer.h"

#include <bit>
#include <fstream>
#include <iterator>
#include <sstream>

#include "googletest/include/gtest/gtest.h"

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

TEST(BigEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(plyodine::WriteToBigEndian(output, {}));

  std::ifstream input("plyodine/test_data/ply_big_empty.ply");
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
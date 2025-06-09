#include "plyodine/writers/in_memory_writer.h"

#include <bit>
#include <cstdint>
#include <iterator>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace plyodine {
namespace {

using ::testing::StartsWith;

TEST(List, UInt8) {
  std::vector<uint8_t> values(std::numeric_limits<uint8_t>::max(),
                              std::numeric_limits<uint8_t>::max());
  std::vector<std::span<const uint8_t>> l0 = {{values.begin(), values.end()}};

  InMemoryWriter writer;
  writer.AddPropertyList("vertex", "l0", l0);

  std::stringstream output;
  ASSERT_EQ(0, writer.WriteToASCII(output).value());

  EXPECT_THAT(output.str(),
              StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                         "list uchar uchar l0\rend_header\r"));
}

TEST(List, UInt16) {
  std::vector<uint8_t> values(std::numeric_limits<uint16_t>::max(),
                              std::numeric_limits<uint8_t>::max());
  std::vector<std::span<const uint8_t>> l0 = {{values.begin(), values.end()}};

  InMemoryWriter writer;
  writer.AddPropertyList("vertex", "l0", l0);

  std::stringstream output;
  ASSERT_EQ(0, writer.WriteToASCII(output).value());

  EXPECT_THAT(output.str(),
              StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                         "list ushort uchar l0\rend_header\r"));
}

TEST(List, UInt32) {
  std::vector<uint8_t> values(std::numeric_limits<uint16_t>::max() + 1u,
                              std::numeric_limits<uint8_t>::max());
  std::vector<std::span<const uint8_t>> l0 = {{values.begin(), values.end()}};

  InMemoryWriter writer;
  writer.AddPropertyList("vertex", "l0", l0);

  std::stringstream output;
  ASSERT_EQ(0, writer.WriteToASCII(output).value());

  EXPECT_THAT(output.str(),
              StartsWith("ply\rformat ascii 1.0\relement vertex 1\rproperty "
                         "list uint uchar l0\rend_header\r"));
}

TEST(ASCII, WithData) {
  std::vector<int8_t> a = {-1, 2, 0};
  std::vector<uint8_t> b = {1u, 2u, 0u};
  std::vector<int16_t> c = {-1, 2, 0};
  std::vector<uint16_t> d = {1u, 2u, 0u};
  std::vector<int32_t> e = {-1, 2, 0};
  std::vector<uint32_t> f = {1u, 2u, 0u};
  std::vector<float> g = {1.5, 2.5, std::acos(-1.0f)};
  std::vector<double> h = {1.5, 2.5, std::acos(-1.0)};
  std::vector<std::span<const int8_t>> al = {{a}};
  std::vector<std::span<const uint8_t>> bl = {{b}};
  std::vector<std::vector<int16_t>> cl = {{c}};
  std::vector<std::vector<uint16_t>> dl = {{d}};
  std::vector<std::vector<int32_t>> el = {{e}};
  std::vector<std::vector<uint32_t>> fl = {{f}};
  std::vector<std::vector<float>> gl = {{g}};
  std::vector<std::vector<double>> hl = {{h}};

  InMemoryWriter writer;
  writer.AddPropertyShallow("vertex", "a", a);
  writer.AddProperty("vertex", "b", b);
  writer.AddProperty("vertex", "c", std::move(c));
  writer.AddProperty("vertex", "d", std::move(d));
  writer.AddProperty("vertex", "e", std::move(e));
  writer.AddProperty("vertex", "f", std::move(f));
  writer.AddProperty("vertex", "g", std::move(g));
  writer.AddProperty("vertex", "h", std::move(h));
  writer.AddPropertyListShallow("vertex_lists", "a", al);
  writer.AddPropertyList("vertex_lists", "b", bl);
  writer.AddPropertyList("vertex_lists", "c", cl);
  writer.AddPropertyList("vertex_lists", "d", dl);
  writer.AddPropertyList("vertex_lists", "e", el);
  writer.AddPropertyList("vertex_lists", "f", std::move(fl));
  writer.AddPropertyList("vertex_lists", "g", std::move(gl));
  writer.AddPropertyList("vertex_lists", "h", std::move(hl));

  writer.AddComment("comment 1");
  writer.AddComment("comment 2");

  writer.AddObjectInfo("obj info 1");
  writer.AddObjectInfo("obj info 2");

  std::stringstream output;
  ASSERT_EQ(0, writer.WriteToASCII(output).value());

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

}  // namespace
}  // namespace plyodine

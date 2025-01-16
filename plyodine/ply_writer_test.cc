#include "plyodine/ply_writer.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <generator>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

#include "googletest/include/gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace plyodine {
namespace {

using ::bazel::tools::cpp::runfiles::Runfiles;

std::ifstream OpenRunfile(const std::string& path) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  return std::ifstream(runfiles->Rlocation(path),
                       std::ios::in | std::ios::binary);
}

struct Property final
    : public std::variant<
          std::span<const int8_t>, std::span<const std::span<const int8_t>>,
          std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
          std::span<const int16_t>, std::span<const std::span<const int16_t>>,
          std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
          std::span<const int32_t>, std::span<const std::span<const int32_t>>,
          std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
          std::span<const float>, std::span<const std::span<const float>>,
          std::span<const double>, std::span<const std::span<const double>>> {
  using std::variant<
      std::span<const int8_t>, std::span<const std::span<const int8_t>>,
      std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
      std::span<const int16_t>, std::span<const std::span<const int16_t>>,
      std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
      std::span<const int32_t>, std::span<const std::span<const int32_t>>,
      std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
      std::span<const float>, std::span<const std::span<const float>>,
      std::span<const double>,
      std::span<const std::span<const double>>>::variant;

  size_t size() const {
    return std::visit([](const auto& entry) { return entry.size(); }, *this);
  }
};

class EmptyWriter final : public PlyWriter {};

class DefaultSizeWriter final : public PlyWriter {
  std::generator<std::span<const uint8_t>> Generator() const {
    for (const auto& value : a_) {
      co_yield std::span(value);
    }
  }

  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override {
    num_element_instances["vertex"] = 1;
    callbacks["vertex"].try_emplace("a", Generator());
    return std::error_code();
  }

  std::vector<std::vector<uint8_t>> a_ = {{1}};
};

class DelegatedWriter final : public PlyWriter {
  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override {
    return std::error_code(2, std::generic_category());
  }
};

class TestWriter final : public PlyWriter {
 public:
  TestWriter(
      const std::map<std::string, std::map<std::string, Property>>& properties,
      std::span<const std::string> comments,
      std::span<const std::string> object_info, bool start_fails = false,
      bool insert_invalid_element = false, bool should_delegate = false,
      uint32_t max_size = std::numeric_limits<uint32_t>::max())
      : properties_(properties),
        comments_(comments),
        object_info_(object_info),
        start_fails_(start_fails),
        insert_invalid_element_(insert_invalid_element),
        should_delegate_(should_delegate),
        max_size_(max_size) {}

  std::unique_ptr<const PlyWriter> DelegateTo() const {
    if (!should_delegate_) {
      return nullptr;
    }

    return std::make_unique<DelegatedWriter>();
  }

  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override {
    if (start_fails_) {
      return std::error_code(1, std::generic_category());
    }

    if (insert_invalid_element_) {
      callbacks.try_emplace("INVALID");
    }

    for (const auto& [element_name, element_properties] : properties_) {
      auto& property_callbacks = callbacks[element_name];
      auto& num_instances = num_element_instances[element_name];
      for (const auto& [property_name, property] : element_properties) {
        num_instances = std::max(num_instances, property.size());
        switch (property.index()) {
          case 0:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<int8_t>(element_name, property_name));
            break;
          case 1:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::ListGenerator<int8_t>(element_name, property_name));
            break;
          case 2:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<uint8_t>(element_name, property_name));
            break;
          case 3:
            property_callbacks.try_emplace(property_name,
                                           TestWriter::ListGenerator<uint8_t>(
                                               element_name, property_name));
            break;
          case 4:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<int16_t>(element_name, property_name));
            break;
          case 5:
            property_callbacks.try_emplace(property_name,
                                           TestWriter::ListGenerator<int16_t>(
                                               element_name, property_name));
            break;
          case 6:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<uint16_t>(element_name, property_name));
            break;
          case 7:
            property_callbacks.try_emplace(property_name,
                                           TestWriter::ListGenerator<uint16_t>(
                                               element_name, property_name));
            break;
          case 8:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<int32_t>(element_name, property_name));
            break;
          case 9:
            property_callbacks.try_emplace(property_name,
                                           TestWriter::ListGenerator<int32_t>(
                                               element_name, property_name));
            break;
          case 10:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<uint32_t>(element_name, property_name));
            break;
          case 11:
            property_callbacks.try_emplace(property_name,
                                           TestWriter::ListGenerator<uint32_t>(
                                               element_name, property_name));
            break;
          case 12:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<float>(element_name, property_name));
            break;
          case 13:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::ListGenerator<float>(element_name, property_name));
            break;
          case 14:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::Generator<double>(element_name, property_name));
            break;
          case 15:
            property_callbacks.try_emplace(
                property_name,
                TestWriter::ListGenerator<double>(element_name, property_name));
            break;
        };
      }
    }

    comments.insert(comments.end(), comments_.begin(), comments_.end());
    object_info.insert(object_info.end(), object_info_.begin(),
                       object_info_.end());

    return std::error_code();
  }

  ListSizeType GetPropertyListSizeType(
      const std::string& element_name,
      const std::string& property_name) const override {
    size_t max_size = std::visit(
        [&](const auto& entry) -> size_t {
          size_t value = 0u;
          if constexpr (std::is_class_v<std::decay_t<decltype(entry[0])>>) {
            for (const auto& list : entry) {
              value = std::max(value, list.size());
            }
          }
          return value;
        },
        properties_.at(element_name).at(property_name));

    max_size = std::min(max_size, static_cast<size_t>(max_size_));

    if (max_size <= std::numeric_limits<uint8_t>::max()) {
      return PlyWriter::ListSizeType::UCHAR;
    }

    if (max_size <= std::numeric_limits<uint16_t>::max()) {
      return PlyWriter::ListSizeType::USHORT;
    }

    return PlyWriter::ListSizeType::UINT;
  }

 private:
  template <typename T>
  std::generator<T> Generator(const std::string& element_name,
                              const std::string& property_name) const {
    const std::span<const T>& values = std::get<std::span<const T>>(
        properties_.at(element_name).at(property_name));
    for (const auto& value : values) {
      co_yield value;
    }
  }

  template <typename T>
  std::generator<std::span<const T>> ListGenerator(
      const std::string& element_name, const std::string& property_name) const {
    const std::span<const std::span<const T>>& values =
        std::get<std::span<const std::span<const T>>>(
            properties_.at(element_name).at(property_name));
    for (const auto& value : values) {
      co_yield value;
    }
  }

  const std::map<std::string, std::map<std::string, Property>>& properties_;
  std::span<const std::string> comments_;
  std::span<const std::string> object_info_;
  bool start_fails_;
  bool insert_invalid_element_;
  bool should_delegate_;
  uint32_t max_size_;
};

std::error_code WriteTo(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteTo(stream);
}

std::error_code WriteToASCII(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToASCII(stream);
}

std::error_code WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToBigEndian(stream);
}

std::error_code WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToLittleEndian(stream);
}

std::map<std::string, std::map<std::string, Property>> BuildTestData() {
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

  std::map<std::string, std::map<std::string, Property>> result;
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

std::map<std::string, std::map<std::string, Property>> BuildListSizeTestData() {
  static const std::vector<uint8_t> values(
      std::numeric_limits<uint16_t>::max() + 1u, 0x88);
  static const std::vector<std::span<const uint8_t>> l0 = {
      {values.begin(), values.begin() + std::numeric_limits<uint8_t>::max()}};
  static const std::vector<std::span<const uint8_t>> l1 = {
      {values.begin(),
       values.begin() + std::numeric_limits<uint8_t>::max() + 1u}};
  static const std::vector<std::span<const uint8_t>> l2 = {
      {values.begin(), values.begin() + std::numeric_limits<uint16_t>::max()}};
  static const std::vector<std::span<const uint8_t>> l3 = {
      {values.begin(), values.end()}};

  std::map<std::string, std::map<std::string, Property>> result;
  result["vertex"]["l0"] = l0;
  result["vertex"]["l1"] = l1;
  result["vertex"]["l2"] = l2;
  result["vertex"]["l3"] = l3;
  return result;
}

TEST(Validate, DefaultErrorCondition) {
  TestWriter writer({}, {}, {}, true);
  std::stringstream output(std::ios::out | std::ios::binary);
  output.clear(std::ios::badbit);

  const std::error_category& error_catgegory =
      writer.WriteTo(output).category();
  EXPECT_NE(error_catgegory.default_error_condition(0),
            std::errc::invalid_argument);
  for (int i = 1; i <= 14; i++) {
    EXPECT_EQ(error_catgegory.default_error_condition(i),
              std::errc::invalid_argument);
  }
  EXPECT_NE(error_catgegory.default_error_condition(15),
            std::errc::invalid_argument);
}

TEST(Validate, BadStream) {
  TestWriter writer({}, {}, {}, true);
  std::stringstream output(std::ios::out | std::ios::binary);
  output.clear(std::ios::badbit);

  EXPECT_EQ("The stream was not in 'good' state",
            writer.WriteTo(output).message());
  EXPECT_EQ("The stream was not in 'good' state",
            writer.WriteToASCII(output).message());
  EXPECT_EQ("The stream was not in 'good' state",
            writer.WriteToBigEndian(output).message());
  EXPECT_EQ("The stream was not in 'good' state",
            writer.WriteToLittleEndian(output).message());
}

TEST(Validate, Delegates) {
  TestWriter writer({}, {}, {}, false, false, true);
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(2, writer.WriteTo(output).value());
  EXPECT_EQ(2, writer.WriteToASCII(output).value());
  EXPECT_EQ(2, writer.WriteToBigEndian(output).value());
  EXPECT_EQ(2, writer.WriteToLittleEndian(output).value());
}

TEST(Validate, StartFails) {
  TestWriter writer({}, {}, {}, true);
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(1, writer.WriteTo(output).value());
  EXPECT_EQ(1, writer.WriteToASCII(output).value());
  EXPECT_EQ(1, writer.WriteToBigEndian(output).value());
  EXPECT_EQ(1, writer.WriteToLittleEndian(output).value());
}

TEST(Validate, BadElementNames) {
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, {{"", {{"prop", {}}}}}).message(),
            "An element had an empty name");
  EXPECT_EQ(WriteToASCII(output, {{" ", {{"prop", {}}}}}).message(),
            "An element name contained invalid characters (must contain only "
            "ASCII graphic characters)");
}

TEST(Validate, EmptyPropertyNames) {
  static const std::vector<int8_t> a = {-1, 2, 0};

  std::map<std::string, std::map<std::string, Property>> properties;
  properties["vertex"][""] = a;

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, properties).message(),
            "A property had an empty name");
}

TEST(Validate, NonGraphicPropertyNames) {
  static const std::vector<int8_t> a = {-1, 2, 0};

  std::map<std::string, std::map<std::string, Property>> properties;
  properties["vertex"][" "] = a;

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, properties).message(),
            "A property name contained invalid characters (must contain only "
            "ASCII graphic characters)");
}

TEST(Validate, BadComment) {
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, {}, {{"\r"}}).message(),
            "A comment string contained invalid characters (must contain only "
            "ASCII space and ASCII graphic characters)");
  EXPECT_EQ(WriteToASCII(output, {}, {{"\n"}}).message(),
            "A comment string contained invalid characters (must contain only "
            "ASCII space and ASCII graphic characters)");
}

TEST(Validate, BadObjInfo) {
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, {}, {}, {{"\r"}}).message(),
            "An obj_info string contained invalid characters (must contain "
            "only ASCII space and ASCII graphic characters)");
  EXPECT_EQ(WriteToASCII(output, {}, {}, {{"\n"}}).message(),
            "An obj_info string contained invalid characters (must contain "
            "only ASCII space and ASCII graphic characters)");
}

TEST(Validate, ListTooBigUInt8) {
  float value;
  std::span<const float> entries(
      &value, static_cast<size_t>(std::numeric_limits<uint8_t>::max()) +
                  static_cast<size_t>(1u));
  std::vector<std::span<const float>> list({entries});

  std::stringstream output(std::ios::out | std::ios::binary);

  std::map<std::string, std::map<std::string, Property>> properties = {
      {"element", {{"node0", {list}}}}};
  TestWriter writer(properties, {}, {}, false, false, false,
                    std::numeric_limits<uint8_t>::max());
  EXPECT_EQ(writer.WriteToASCII(output).message(),
            "A property list with size type 'uchar' exceeded its maximum "
            "supported length (255 entries)");
}

TEST(Validate, ListTooBigUInt16) {
  float value;
  std::span<const float> entries(
      &value, static_cast<size_t>(std::numeric_limits<uint16_t>::max()) +
                  static_cast<size_t>(1u));
  std::vector<std::span<const float>> list({entries});

  std::stringstream output(std::ios::out | std::ios::binary);

  std::map<std::string, std::map<std::string, Property>> properties = {
      {"element", {{"node0", {list}}}}};
  TestWriter writer(properties, {}, {}, false, false, false,
                    std::numeric_limits<uint16_t>::max());
  EXPECT_EQ(writer.WriteToASCII(output).message(),
            "A property list with size type 'ushort' exceeded its maximum "
            "supported length (65,535 entries)");
}

TEST(Validate, ListTooBigUInt32) {
  if constexpr (std::numeric_limits<uint32_t>::max() <
                std::numeric_limits<size_t>::max()) {
    float value;
    std::span<const float> entries(
        &value, static_cast<size_t>(std::numeric_limits<uint32_t>::max()) +
                    static_cast<size_t>(1u));
    std::vector<std::span<const float>> list({entries});

    std::stringstream output(std::ios::out | std::ios::binary);
    EXPECT_EQ(
        WriteToASCII(output, {{"element", {{"node0", {list}}}}}).message(),
        "A property list with size type 'uint' exceeded its maximum supported "
        "length (4,294,967,295 entries)");
  }
}

TEST(Validate, UnbalancedProperties) {
  static const std::vector<int8_t> a = {-1, 2, 0};
  static const std::vector<uint8_t> b = {1u, 2u};

  std::map<std::string, std::map<std::string, Property>> properties;
  properties["vertex"]["a"] = a;
  properties["vertex"]["b"] = b;

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, properties).message(),
            "A property generator did not produce enough data for all "
            "instances of its element");
}

TEST(All, DefaultListSize) {
  DefaultSizeWriter writer;
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(0, writer.WriteToASCII(output).value());

  std::stringstream input(
      "ply\r"
      "format ascii 1.0\r"
      "element vertex 1\r"
      "property list uint uchar a\r"
      "end_header\r"
      "1 1\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(All, NoInstances) {
  std::map<std::string, std::map<std::string, Property>> properties;
  properties["vertex"]["a"] = std::vector<int8_t>();

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(0, WriteToASCII(output, properties).value());

  std::stringstream input(
      "ply\r"
      "format ascii 1.0\r"
      "element vertex 0\r"
      "property char a\r"
      "end_header\r");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(All, NoProperties) {
  TestWriter writer({}, {}, {}, false, true);
  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(writer.WriteTo(output).message(), "An element had no properties");
}

TEST(ASCII, Empty) {
  EmptyWriter writer;
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(writer.WriteToASCII(output).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, NonFinite) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::map<std::string, std::map<std::string, Property>> data;
  data["vertex"]["a"] = a;

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(WriteToASCII(output, data).message(),
            "A non-finite floating-point property value cannot be written to "
            "an ASCII output");
}

TEST(ASCII, NonFiniteList) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::vector<std::span<const float>> al = {{a}};
  std::map<std::string, std::map<std::string, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output(std::ios::out | std::ios::binary);
  EXPECT_EQ(
      WriteToASCII(output, data).message(),
      "A non-finite floating-point property list value cannot be written to "
      "an ASCII output");
}

TEST(ASCII, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(
      WriteToASCII(output, BuildTestData(), comments, object_info).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, ListSizes) {
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToASCII(output, BuildListSizeTestData()).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, LargeFP) {
  std::vector<double> a = {18446744073709551616.0};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string, std::map<std::string, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToASCII(output, data).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_large_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, SmallFP) {
  std::vector<double> a = {0.000000000000000000000025};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string, std::map<std::string, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToASCII(output, data).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_ascii_small_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, Empty) {
  EmptyWriter writer;
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(writer.WriteToBigEndian(output).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_big_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(
      WriteToBigEndian(output, BuildTestData(), comments, object_info).value(),
      0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, ListSizes) {
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToBigEndian(output, BuildListSizeTestData()).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, Empty) {
  EmptyWriter writer;
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(writer.WriteToLittleEndian(output).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_little_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToLittleEndian(output, BuildTestData(), comments, object_info)
                .value(),
            0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, ListSizes) {
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteToLittleEndian(output, BuildListSizeTestData()).value(), 0);

  std::ifstream input =
      OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(Native, Empty) {
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteTo(output, {}).value(), 0);

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_big_empty.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_little_empty.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}

TEST(Native, TestData) {
  std::stringstream output(std::ios::out | std::ios::binary);
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  ASSERT_EQ(WriteTo(output, BuildTestData(), comments, object_info).value(), 0);

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_big_data.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_little_data.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}

TEST(Native, ListSizes) {
  std::stringstream output(std::ios::out | std::ios::binary);
  ASSERT_EQ(WriteTo(output, BuildListSizeTestData()).value(), 0);

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_big_list_sizes.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input =
        OpenRunfile("_main/plyodine/test_data/ply_little_list_sizes.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}

}  // namespace
}  // namespace plyodine
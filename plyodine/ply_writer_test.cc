#include "plyodine/ply_writer.h"

#include <bit>
#include <cmath>
#include <fstream>
#include <iterator>
#include <sstream>

#include "googletest/include/gtest/gtest.h"

struct Property final
    : public std::variant<std::span<const plyodine::Int8Property>,
                          std::span<const plyodine::Int8PropertyList>,
                          std::span<const plyodine::UInt8Property>,
                          std::span<const plyodine::UInt8PropertyList>,
                          std::span<const plyodine::Int16Property>,
                          std::span<const plyodine::Int16PropertyList>,
                          std::span<const plyodine::UInt16Property>,
                          std::span<const plyodine::UInt16PropertyList>,
                          std::span<const plyodine::Int32Property>,
                          std::span<const plyodine::Int32PropertyList>,
                          std::span<const plyodine::UInt32Property>,
                          std::span<const plyodine::UInt32PropertyList>,
                          std::span<const plyodine::FloatProperty>,
                          std::span<const plyodine::FloatPropertyList>,
                          std::span<const plyodine::DoubleProperty>,
                          std::span<const plyodine::DoublePropertyList>> {
  using std::variant<std::span<const plyodine::Int8Property>,
                     std::span<const plyodine::Int8PropertyList>,
                     std::span<const plyodine::UInt8Property>,
                     std::span<const plyodine::UInt8PropertyList>,
                     std::span<const plyodine::Int16Property>,
                     std::span<const plyodine::Int16PropertyList>,
                     std::span<const plyodine::UInt16Property>,
                     std::span<const plyodine::UInt16PropertyList>,
                     std::span<const plyodine::Int32Property>,
                     std::span<const plyodine::Int32PropertyList>,
                     std::span<const plyodine::UInt32Property>,
                     std::span<const plyodine::UInt32PropertyList>,
                     std::span<const plyodine::FloatProperty>,
                     std::span<const plyodine::FloatPropertyList>,
                     std::span<const plyodine::DoubleProperty>,
                     std::span<const plyodine::DoublePropertyList>>::variant;

  constexpr plyodine::PropertyType type() const {
    return static_cast<plyodine::PropertyType>(index());
  }

  size_t size() const {
    return std::visit([](const auto& entry) { return entry.size(); }, *this);
  }
};

class TestWriter final : public plyodine::PlyWriter {
 public:
  TestWriter(const std::map<std::string_view,
                            std::map<std::string_view, Property>>& properties,
             std::span<const std::string> comments,
             std::span<const std::string> object_info, bool start_fails = false)
      : properties_(properties),
        comments_(comments),
        object_info_(object_info),
        start_fails_(start_fails) {}

  std::expected<void, std::string_view> Start(
      std::map<std::string_view,
               std::pair<uint64_t, std::map<std::string_view, Callback>>>&
          property_callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override {
    if (start_fails_) {
      return std::unexpected("start");
    }

    for (const auto& element : properties_) {
      uint64_t num_properties = 0u;
      std::map<std::string_view, PlyWriter::Callback> callbacks;
      for (const auto& property : element.second) {
        num_properties = property.second.size();
        switch (property.second.type()) {
          case plyodine::PropertyType::INT8:
            callbacks[property.first] = Int8PropertyCallback(
                &TestWriter::Callback<plyodine::Int8Property>);
            break;
          case plyodine::PropertyType::INT8_LIST:
            callbacks[property.first] = Int8PropertyListCallback(
                &TestWriter::ListCallback<plyodine::Int8Property>);
            break;
          case plyodine::PropertyType::UINT8:
            callbacks[property.first] = UInt8PropertyCallback(
                &TestWriter::Callback<plyodine::UInt8Property>);
            break;
          case plyodine::PropertyType::UINT8_LIST:
            callbacks[property.first] = UInt8PropertyListCallback(
                &TestWriter::ListCallback<plyodine::UInt8Property>);
            break;
          case plyodine::PropertyType::INT16:
            callbacks[property.first] = Int16PropertyCallback(
                &TestWriter::Callback<plyodine::Int16Property>);
            break;
          case plyodine::PropertyType::INT16_LIST:
            callbacks[property.first] = Int16PropertyListCallback(
                &TestWriter::ListCallback<plyodine::Int16Property>);
            break;
          case plyodine::PropertyType::UINT16:
            callbacks[property.first] = UInt16PropertyCallback(
                &TestWriter::Callback<plyodine::UInt16Property>);
            break;
          case plyodine::PropertyType::UINT16_LIST:
            callbacks[property.first] = UInt16PropertyListCallback(
                &TestWriter::ListCallback<plyodine::UInt16Property>);
            break;
          case plyodine::PropertyType::INT32:
            callbacks[property.first] = Int32PropertyCallback(
                &TestWriter::Callback<plyodine::Int32Property>);
            break;
          case plyodine::PropertyType::INT32_LIST:
            callbacks[property.first] = Int32PropertyListCallback(
                &TestWriter::ListCallback<plyodine::Int32Property>);
            break;
          case plyodine::PropertyType::UINT32:
            callbacks[property.first] = UInt32PropertyCallback(
                &TestWriter::Callback<plyodine::UInt32Property>);
            break;
          case plyodine::PropertyType::UINT32_LIST:
            callbacks[property.first] = UInt32PropertyListCallback(
                &TestWriter::ListCallback<plyodine::UInt32Property>);
            break;
          case plyodine::PropertyType::FLOAT:
            callbacks[property.first] = FloatPropertyCallback(
                &TestWriter::Callback<plyodine::FloatProperty>);
            break;
          case plyodine::PropertyType::FLOAT_LIST:
            callbacks[property.first] = FloatPropertyListCallback(
                &TestWriter::ListCallback<plyodine::FloatProperty>);
            break;
          case plyodine::PropertyType::DOUBLE:
            callbacks[property.first] = DoublePropertyCallback(
                &TestWriter::Callback<plyodine::DoubleProperty>);
            break;
          case plyodine::PropertyType::DOUBLE_LIST:
            callbacks[property.first] = DoublePropertyListCallback(
                &TestWriter::ListCallback<plyodine::DoubleProperty>);
            break;
        };
      }

      property_callbacks[element.first] =
          std::make_pair(num_properties, std::move(callbacks));
    }

    comments.insert(comments.end(), comments_.begin(), comments_.end());
    object_info.insert(object_info.end(), object_info_.begin(),
                       object_info_.end());

    return std::expected<void, std::string_view>();
  }

  std::expected<ListSizeType, std::string_view> GetPropertyListSizeType(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index) const override {
    size_t max_size = std::visit(
        [&](const auto& entry) -> size_t {
          size_t value = 0u;
          if constexpr (std::is_class<
                            std::decay_t<decltype(entry[0])>>::value) {
            for (const auto& list : entry) {
              value = std::max(value, list.size());
            }
          }
          return value;
        },
        properties_.at(element_name).at(property_name));

    if (max_size <= std::numeric_limits<uint8_t>::max()) {
      return PlyWriter::ListSizeType::UINT8;
    }

    if (max_size <= std::numeric_limits<uint16_t>::max()) {
      return PlyWriter::ListSizeType::UINT16;
    }

    return PlyWriter::ListSizeType::UINT32;
  }

 private:
  template <typename T>
  std::expected<T, std::string_view> Callback(std::string_view element_name,
                                              size_t element_index,
                                              std::string_view property_name,
                                              size_t property_index,
                                              uint64_t instance) const {
    return std::get<std::span<const T>>(
        properties_.at(element_name).at(property_name))[instance];
  }

  template <typename T>
  std::expected<std::span<const T>, std::string_view> ListCallback(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index, uint64_t instance,
      std::vector<T>& storage) const {
    return std::get<std::span<const std::span<const T>>>(
        properties_.at(element_name).at(property_name))[instance];
  }

  const std::map<std::string_view, std::map<std::string_view, Property>>&
      properties_;
  std::span<const std::string> comments_;
  std::span<const std::string> object_info_;
  bool start_fails_;
};

std::expected<void, std::string_view> WriteTo(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteTo(stream);
}

std::expected<void, std::string_view> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToASCII(stream);
}

std::expected<void, std::string_view> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToBigEndian(stream);
}

std::expected<void, std::string_view> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments = {},
    std::span<const std::string> object_info = {}) {
  TestWriter writer(properties, comments, object_info);
  return writer.WriteToLittleEndian(stream);
}

std::map<std::string_view, std::map<std::string_view, Property>>
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

  std::map<std::string_view, std::map<std::string_view, Property>> result;
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

std::map<std::string_view, std::map<std::string_view, Property>>
BuildListSizeTestData() {
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

  std::map<std::string_view, std::map<std::string_view, Property>> result;
  result["vertex"]["l0"] = l0;
  result["vertex"]["l1"] = l1;
  result["vertex"]["l2"] = l2;
  result["vertex"]["l3"] = l3;
  return result;
}

TEST(Validate, StartFails) {
  TestWriter writer({}, {}, {}, true);
  std::stringstream output;
  EXPECT_EQ(writer.WriteTo(output).error(), "start");
  EXPECT_EQ(writer.WriteToASCII(output).error(), "start");
  EXPECT_EQ(writer.WriteToBigEndian(output).error(), "start");
  EXPECT_EQ(writer.WriteToLittleEndian(output).error(), "start");
}

TEST(Validate, BadElementNames) {
  std::stringstream output;
  EXPECT_EQ(WriteToASCII(output, {{"", {}}}).error(),
            "Names of properties and elements may not be empty");
  EXPECT_EQ(
      WriteToASCII(output, {{" ", {}}}).error(),
      "Names of properties and elements may only contain graphic characters");
}

TEST(Validate, BadPropertyNames) {
  std::stringstream output;
  EXPECT_EQ(WriteToASCII(output, {{"element", {{"", {}}}}}).error(),
            "Names of properties and elements may not be empty");
  EXPECT_EQ(
      WriteToASCII(output, {{"element", {{" ", {}}}}}).error(),
      "Names of properties and elements may only contain graphic characters");
}

TEST(Validate, BadComment) {
  std::stringstream output;
  EXPECT_EQ(WriteToASCII(output, {}, {{"\r"}}).error(),
            "A comment may not contain line feed or carriage return");
  EXPECT_EQ(WriteToASCII(output, {}, {{"\n"}}).error(),
            "A comment may not contain line feed or carriage return");
}

TEST(Validate, BadObjInfo) {
  std::stringstream output;
  EXPECT_EQ(WriteToASCII(output, {}, {}, {{"\r"}}).error(),
            "An obj_info may not contain line feed or carriage return");
  EXPECT_EQ(WriteToASCII(output, {}, {}, {{"\n"}}).error(),
            "An obj_info may not contain line feed or carriage return");
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
    EXPECT_EQ(
        WriteToASCII(output, {{"element", {{"node0", {list}}}}}).error(),
        "The list was too big to be represented with the selected size type");
  }
}

TEST(ASCII, Empty) {
  std::stringstream output;
  ASSERT_TRUE(WriteToASCII(output, {}));

  std::ifstream input("plyodine/test_data/ply_ascii_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, NonFinite) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::map<std::string_view, std::map<std::string_view, Property>> data;
  data["vertex"]["a"] = a;

  std::stringstream output;
  EXPECT_EQ(
      WriteToASCII(output, data).error(),
      "Only finite floating point values may be serialized to an ASCII output");
}

TEST(ASCII, NonFiniteList) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::vector<std::span<const float>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  EXPECT_EQ(
      WriteToASCII(output, data).error(),
      "Only finite floating point values may be serialized to an ASCII output");
}

TEST(ASCII, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output;
  ASSERT_TRUE(WriteToASCII(output, BuildTestData(), comments, object_info));

  std::ifstream input("plyodine/test_data/ply_ascii_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, ListSizes) {
  std::stringstream output;
  ASSERT_TRUE(WriteToASCII(output, BuildListSizeTestData()));

  std::ifstream input("plyodine/test_data/ply_ascii_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, LargeFP) {
  std::vector<double> a = {18446744073709551616.0};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  ASSERT_TRUE(WriteToASCII(output, data));

  std::ifstream input("plyodine/test_data/ply_ascii_large_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(ASCII, SmallFP) {
  std::vector<double> a = {0.000000000000000000000025};
  std::vector<std::span<const double>> al = {{a}};
  std::map<std::string_view, std::map<std::string_view, Property>> data;
  data["vertex"]["a"] = al;

  std::stringstream output;
  ASSERT_TRUE(WriteToASCII(output, data));

  std::ifstream input("plyodine/test_data/ply_ascii_small_fp.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(WriteToBigEndian(output, {}));

  std::ifstream input("plyodine/test_data/ply_big_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output;
  ASSERT_TRUE(WriteToBigEndian(output, BuildTestData(), comments, object_info));

  std::ifstream input("plyodine/test_data/ply_big_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(BigEndian, ListSizes) {
  std::stringstream output;
  ASSERT_TRUE(WriteToBigEndian(output, BuildListSizeTestData()));

  std::ifstream input("plyodine/test_data/ply_big_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, Empty) {
  std::stringstream output;
  ASSERT_TRUE(WriteToLittleEndian(output, {}));

  std::ifstream input("plyodine/test_data/ply_little_empty.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, TestData) {
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  std::stringstream output;
  ASSERT_TRUE(
      WriteToLittleEndian(output, BuildTestData(), comments, object_info));

  std::ifstream input("plyodine/test_data/ply_little_data.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(LittleEndian, ListSizes) {
  std::stringstream output;
  ASSERT_TRUE(WriteToLittleEndian(output, BuildListSizeTestData()));

  std::ifstream input("plyodine/test_data/ply_little_list_sizes.ply");
  std::string expected(std::istreambuf_iterator<char>(input), {});
  EXPECT_EQ(expected, output.str());
}

TEST(Native, Empty) {
  std::stringstream output;
  ASSERT_TRUE(WriteTo(output, {}));

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
  std::string comments[] = {{"comment 1"}, {"comment 2"}};
  std::string object_info[] = {{"obj info 1"}, {"obj info 2"}};
  ASSERT_TRUE(WriteTo(output, BuildTestData(), comments, object_info));

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

TEST(Native, ListSizes) {
  std::stringstream output;
  ASSERT_TRUE(WriteTo(output, BuildListSizeTestData()));

  if constexpr (std::endian::native == std::endian::big) {
    std::ifstream input("plyodine/test_data/ply_big_list_sizes.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  } else {
    std::ifstream input("plyodine/test_data/ply_little_list_sizes.ply");
    std::string expected(std::istreambuf_iterator<char>(input), {});
    EXPECT_EQ(expected, output.str());
  }
}
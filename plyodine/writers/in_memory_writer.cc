#include "plyodine/writers/in_memory_writer.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "plyodine/ply_writer.h"

namespace plyodine {
namespace {

class InMemoryWriter final : public PlyWriter {
 public:
  InMemoryWriter(
      const std::map<std::string_view, std::map<std::string_view, Property>>&
          properties,
      std::span<const std::string> comments,
      std::span<const std::string> object_info);

  std::expected<void, std::string_view> Start(
      std::map<std::string_view,
               std::pair<uint64_t, std::map<std::string_view, Callback>>>&
          property_callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override;

  std::expected<ListSizeType, std::string_view> GetPropertyListSizeType(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index) const override;

 private:
  template <typename T>
  std::expected<T, std::string_view> Callback(std::string_view element_name,
                                              size_t element_index,
                                              std::string_view property_name,
                                              size_t property_index,
                                              uint64_t instance) const;

  template <typename T>
  std::expected<std::span<const T>, std::string_view> ListCallback(
      std::string_view element_name, size_t element_index,
      std::string_view property_name, size_t property_index, uint64_t instance,
      std::vector<T>& storage) const;

  const std::map<std::string_view, std::map<std::string_view, Property>>&
      properties_;
  std::span<const std::string> comments_;
  std::span<const std::string> object_info_;

  std::vector<std::pair<uint64_t, std::vector<const Property*>>>
      indexed_properties_;
};

InMemoryWriter::InMemoryWriter(
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info)
    : properties_(properties), comments_(comments), object_info_(object_info) {
  for (const auto& element : properties_) {
    std::vector<const Property*> list;
    size_t num_elements = 0u;
    for (const auto& property : element.second) {
      num_elements = property.second.size();
      list.push_back(&property.second);
    }

    indexed_properties_.emplace_back(num_elements, std::move(list));
  }
}

template <typename T>
std::expected<T, std::string_view> InMemoryWriter::Callback(
    std::string_view element_name, size_t element_index,
    std::string_view property_name, size_t property_index,
    uint64_t instance) const {
  return std::get<std::span<const T>>(
      *indexed_properties_.at(element_index)
           .second.at(property_index))[instance];
}

template <typename T>
std::expected<std::span<const T>, std::string_view>
InMemoryWriter::ListCallback(std::string_view element_name,
                             size_t element_index,
                             std::string_view property_name,
                             size_t property_index, uint64_t instance,
                             std::vector<T>& storage) const {
  return std::get<std::span<const std::span<const T>>>(
      *indexed_properties_.at(element_index)
           .second.at(property_index))[instance];
}

std::expected<void, std::string_view> InMemoryWriter::Start(
    std::map<
        std::string_view,
        std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>&
        property_callbacks,
    std::vector<std::string>& comments,
    std::vector<std::string>& object_info) const {
  for (const auto& element : properties_) {
    std::map<std::string_view, PlyWriter::Callback> callbacks;
    std::optional<size_t> num_elements;
    for (const auto& property : element.second) {
      size_t property_num_elements = property.second.size();

      if (num_elements.has_value()) {
        if (*num_elements != property_num_elements) {
          return std::unexpected(
              "All properties of an element must have the same size");
        }
      } else {
        num_elements = property_num_elements;
      }

      switch (property.second.type()) {
        case PropertyType::INT8:
          callbacks.emplace(
              property.first,
              Int8PropertyCallback(&InMemoryWriter::Callback<Int8Property>));
          break;
        case PropertyType::INT8_LIST:
          callbacks.emplace(property.first,
                            Int8PropertyListCallback(
                                &InMemoryWriter::ListCallback<Int8Property>));
          break;
        case PropertyType::UINT8:
          callbacks.emplace(
              property.first,
              UInt8PropertyCallback(&InMemoryWriter::Callback<UInt8Property>));
          break;
        case PropertyType::UINT8_LIST:
          callbacks.emplace(property.first,
                            UInt8PropertyListCallback(
                                &InMemoryWriter::ListCallback<UInt8Property>));
          break;
        case PropertyType::INT16:
          callbacks.emplace(
              property.first,
              Int16PropertyCallback(&InMemoryWriter::Callback<Int16Property>));
          break;
        case PropertyType::INT16_LIST:
          callbacks.emplace(property.first,
                            Int16PropertyListCallback(
                                &InMemoryWriter::ListCallback<Int16Property>));
          break;
        case PropertyType::UINT16:
          callbacks.emplace(property.first,
                            UInt16PropertyCallback(
                                &InMemoryWriter::Callback<UInt16Property>));
          break;
        case PropertyType::UINT16_LIST:
          callbacks.emplace(property.first,
                            UInt16PropertyListCallback(
                                &InMemoryWriter::ListCallback<UInt16Property>));
          break;
        case PropertyType::INT32:
          callbacks.emplace(
              property.first,
              Int32PropertyCallback(&InMemoryWriter::Callback<Int32Property>));
          break;
        case PropertyType::INT32_LIST:
          callbacks.emplace(property.first,
                            Int32PropertyListCallback(
                                &InMemoryWriter::ListCallback<Int32Property>));
          break;
        case PropertyType::UINT32:
          callbacks.emplace(property.first,
                            UInt32PropertyCallback(
                                &InMemoryWriter::Callback<UInt32Property>));
          break;
        case PropertyType::UINT32_LIST:
          callbacks.emplace(property.first,
                            UInt32PropertyListCallback(
                                &InMemoryWriter::ListCallback<UInt32Property>));
          break;
        case PropertyType::FLOAT:
          callbacks.emplace(
              property.first,
              FloatPropertyCallback(&InMemoryWriter::Callback<FloatProperty>));
          break;
        case PropertyType::FLOAT_LIST:
          callbacks.emplace(property.first,
                            FloatPropertyListCallback(
                                &InMemoryWriter::ListCallback<FloatProperty>));
          break;
        case PropertyType::DOUBLE:
          callbacks.emplace(property.first,
                            DoublePropertyCallback(
                                &InMemoryWriter::Callback<DoubleProperty>));
          break;
        case PropertyType::DOUBLE_LIST:
          callbacks.emplace(property.first,
                            DoublePropertyListCallback(
                                &InMemoryWriter::ListCallback<DoubleProperty>));
          break;
      };
    }

    property_callbacks[element.first] =
        std::make_pair(num_elements.value_or(0), std::move(callbacks));
  }

  comments.insert(comments.end(), comments_.begin(), comments_.end());
  object_info.insert(object_info.end(), object_info_.begin(),
                     object_info_.end());

  return std::expected<void, std::string_view>();
}

std::expected<PlyWriter::ListSizeType, std::string_view>
InMemoryWriter::GetPropertyListSizeType(std::string_view element_name,
                                        size_t element_index,
                                        std::string_view property_name,
                                        size_t property_index) const {
  size_t max_size = std::visit(
      [&](const auto& entry) -> size_t {
        size_t value = 0u;
        if constexpr (std::is_class<std::decay_t<decltype(entry[0])>>::value) {
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

  if (max_size <= std::numeric_limits<uint32_t>::max()) {
    return PlyWriter::ListSizeType::UINT32;
  }

  return std::unexpected(
      "Property lists can contain no more than 4294967295 entries");
}

}  // namespace

std::expected<void, std::string_view> WriteTo(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteTo(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToASCII(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToBigEndian(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string_view> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string_view, std::map<std::string_view, Property>>&
        properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToLittleEndian(stream);
}

// Static asserts for Property variant
static_assert(Property(std::span<const Int8Property>()).index() ==
              static_cast<size_t>(PropertyType::INT8));
static_assert(Property(std::span<const Int8PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::INT8_LIST));
static_assert(Property(std::span<const UInt8Property>()).index() ==
              static_cast<size_t>(PropertyType::UINT8));
static_assert(Property(std::span<const UInt8PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::UINT8_LIST));
static_assert(Property(std::span<const Int16Property>()).index() ==
              static_cast<size_t>(PropertyType::INT16));
static_assert(Property(std::span<const Int16PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::INT16_LIST));
static_assert(Property(std::span<const UInt16Property>()).index() ==
              static_cast<size_t>(PropertyType::UINT16));
static_assert(Property(std::span<const UInt16PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::UINT16_LIST));
static_assert(Property(std::span<const Int32Property>()).index() ==
              static_cast<size_t>(PropertyType::INT32));
static_assert(Property(std::span<const Int32PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::INT32_LIST));
static_assert(Property(std::span<const UInt32Property>()).index() ==
              static_cast<size_t>(PropertyType::UINT32));
static_assert(Property(std::span<const UInt32PropertyList>()).index() ==
              static_cast<size_t>(PropertyType::UINT32_LIST));
static_assert(Property(std::span<const FloatProperty>()).index() ==
              static_cast<size_t>(PropertyType::FLOAT));
static_assert(Property(std::span<const FloatPropertyList>()).index() ==
              static_cast<size_t>(PropertyType::FLOAT_LIST));
static_assert(Property(std::span<const DoubleProperty>()).index() ==
              static_cast<size_t>(PropertyType::DOUBLE));
static_assert(Property(std::span<const DoublePropertyList>()).index() ==
              static_cast<size_t>(PropertyType::DOUBLE_LIST));

}  // namespace plyodine
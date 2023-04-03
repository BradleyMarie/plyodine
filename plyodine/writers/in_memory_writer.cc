#include "plyodine/writers/in_memory_writer.h"

#include <algorithm>
#include <limits>
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
      std::span<const std::string> object_info)
      : properties_(properties),
        comments_(comments),
        object_info_(object_info) {}

  std::expected<void, std::string_view> Start(
      std::map<std::string_view,
               std::pair<uint64_t, std::map<std::string_view, Callback>>>&
          property_callbacks,
      std::span<const std::string>& comments,
      std::span<const std::string>& object_info) override;

  std::expected<SizeType, std::string_view> GetPropertyListSizeType(
      std::string_view element_name,
      std::string_view property_name) const override;

 private:
  template <typename T>
  std::expected<T, std::string_view> Callback(std::string_view element_name,
                                              std::string_view property_name);

  const std::map<std::string_view, std::map<std::string_view, Property>>&
      properties_;
  std::span<const std::string> comments_;
  std::span<const std::string> object_info_;

  std::vector<std::pair<uint64_t, std::vector<const Property*>>>
      indexed_properties_;
  uint64_t element_count = 0u;
  size_t element_index = 0u;
  size_t property_index = 0u;
};

template <typename T>
std::expected<T, std::string_view> InMemoryWriter::Callback(
    std::string_view element_name, std::string_view property_name) {
  const auto& properties = indexed_properties_.at(element_index).second;
  const auto& property = properties.at(property_index++);

  auto value = std::get<std::span<const T>>(*property)[element_count];

  if (property_index == properties.size()) {
    element_count += 1u;
    property_index = 0u;

    if (element_count == indexed_properties_[element_index].first) {
      element_index += 1u;
      element_count = 0u;
    }
  }

  return value;
}

std::expected<void, std::string_view> InMemoryWriter::Start(
    std::map<
        std::string_view,
        std::pair<uint64_t, std::map<std::string_view, PlyWriter::Callback>>>&
        property_callbacks,
    std::span<const std::string>& comments,
    std::span<const std::string>& object_info) {
  for (const auto& element : properties_) {
    std::map<std::string_view, PlyWriter::Callback> callbacks;
    std::vector<const Property*> list;
    size_t num_elements = 0u;
    for (const auto& property : element.second) {
      size_t property_num_elements = property.second.size();

      if (!list.empty()) {
        if (num_elements != property_num_elements) {
          return std::unexpected(
              "All properties of an element must have the same size");
        }
      } else {
        num_elements = property_num_elements;
      }

      switch (property.second.index()) {
        case Property::INT8:
          callbacks.emplace(
              property.first,
              Int8PropertyCallback(&InMemoryWriter::Callback<Int8Property>));
          break;
        case Property::INT8_LIST:
          callbacks.emplace(property.first,
                            Int8PropertyListCallback(
                                &InMemoryWriter::Callback<Int8PropertyList>));
          break;
        case Property::UINT8:
          callbacks.emplace(
              property.first,
              UInt8PropertyCallback(&InMemoryWriter::Callback<UInt8Property>));
          break;
        case Property::UINT8_LIST:
          callbacks.emplace(property.first,
                            UInt8PropertyListCallback(
                                &InMemoryWriter::Callback<UInt8PropertyList>));
          break;
        case Property::INT16:
          callbacks.emplace(
              property.first,
              Int16PropertyCallback(&InMemoryWriter::Callback<Int16Property>));
          break;
        case Property::INT16_LIST:
          callbacks.emplace(property.first,
                            Int16PropertyListCallback(
                                &InMemoryWriter::Callback<Int16PropertyList>));
          break;
        case Property::UINT16:
          callbacks.emplace(property.first,
                            UInt16PropertyCallback(
                                &InMemoryWriter::Callback<UInt16Property>));
          break;
        case Property::UINT16_LIST:
          callbacks.emplace(property.first,
                            UInt16PropertyListCallback(
                                &InMemoryWriter::Callback<UInt16PropertyList>));
          break;
        case Property::INT32:
          callbacks.emplace(
              property.first,
              Int32PropertyCallback(&InMemoryWriter::Callback<Int32Property>));
          break;
        case Property::INT32_LIST:
          callbacks.emplace(property.first,
                            Int32PropertyListCallback(
                                &InMemoryWriter::Callback<Int32PropertyList>));
          break;
        case Property::UINT32:
          callbacks.emplace(property.first,
                            UInt32PropertyCallback(
                                &InMemoryWriter::Callback<UInt32Property>));
          break;
        case Property::UINT32_LIST:
          callbacks.emplace(property.first,
                            UInt32PropertyListCallback(
                                &InMemoryWriter::Callback<UInt32PropertyList>));
          break;
        case Property::FLOAT:
          callbacks.emplace(
              property.first,
              FloatPropertyCallback(&InMemoryWriter::Callback<FloatProperty>));
          break;
        case Property::FLOAT_LIST:
          callbacks.emplace(property.first,
                            FloatPropertyListCallback(
                                &InMemoryWriter::Callback<FloatPropertyList>));
          break;
        case Property::DOUBLE:
          callbacks.emplace(property.first,
                            DoublePropertyCallback(
                                &InMemoryWriter::Callback<DoubleProperty>));
          break;
        case Property::DOUBLE_LIST:
          callbacks.emplace(property.first,
                            DoublePropertyListCallback(
                                &InMemoryWriter::Callback<DoublePropertyList>));
          break;
      };

      list.push_back(&property.second);
    }

    indexed_properties_.emplace_back(num_elements, std::move(list));
    property_callbacks[element.first] =
        std::make_pair(num_elements, std::move(callbacks));
  }
  comments = comments_;
  object_info = object_info_;

  return std::expected<void, std::string_view>();
}

std::expected<PlyWriter::SizeType, std::string_view>
InMemoryWriter::GetPropertyListSizeType(std::string_view element_name,
                                        std::string_view property_name) const {
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
    return PlyWriter::UINT8;
  }

  if (max_size <= std::numeric_limits<uint16_t>::max()) {
    return PlyWriter::UINT16;
  }

  if (max_size <= std::numeric_limits<uint32_t>::max()) {
    return PlyWriter::UINT32;
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

}  // namespace plyodine
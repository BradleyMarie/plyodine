#include "plyodine/writers/in_memory_writer.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "plyodine/ply_writer.h"

namespace plyodine {
namespace {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

class InMemoryWriter final : public PlyWriter {
 public:
  InMemoryWriter(
      const std::map<std::string, std::map<std::string, Property>>& properties,
      std::span<const std::string> comments,
      std::span<const std::string> object_info);

  std::expected<void, std::string> Start(
      std::map<std::string,
               std::pair<uint64_t, std::map<std::string, Callback>>>&
          property_callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override;

  std::expected<ListSizeType, std::string> GetPropertyListSizeType(
      const std::string& element_name, size_t element_index,
      const std::string& property_name, size_t property_index) const override;

 private:
  template <typename T>
  std::expected<T, std::string> Callback(const std::string& element_name,
                                         size_t element_index,
                                         const std::string& property_name,
                                         size_t property_index,
                                         uint64_t instance) const;

  template <typename T>
  std::expected<std::span<const T>, std::string> ListCallback(
      const std::string& element_name, size_t element_index,
      const std::string& property_name, size_t property_index,
      uint64_t instance, std::vector<T>& storage) const;

  const std::map<std::string, std::map<std::string, Property>>& properties_;
  std::span<const std::string> comments_;
  std::span<const std::string> object_info_;

  std::vector<std::pair<uint64_t, std::vector<const Property*>>>
      indexed_properties_;
};

InMemoryWriter::InMemoryWriter(
    const std::map<std::string, std::map<std::string, Property>>& properties,
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
std::expected<T, std::string> InMemoryWriter::Callback(
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index,
    uint64_t instance) const {
  return std::get<std::span<const T>>(
      *indexed_properties_.at(element_index)
           .second.at(property_index))[instance];
}

template <typename T>
std::expected<std::span<const T>, std::string> InMemoryWriter::ListCallback(
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index, uint64_t instance,
    std::vector<T>& storage) const {
  return std::get<std::span<const std::span<const T>>>(
      *indexed_properties_.at(element_index)
           .second.at(property_index))[instance];
}

std::expected<void, std::string> InMemoryWriter::Start(
    std::map<std::string,
             std::pair<uint64_t, std::map<std::string, PlyWriter::Callback>>>&
        property_callbacks,
    std::vector<std::string>& comments,
    std::vector<std::string>& object_info) const {
  for (const auto& element : properties_) {
    std::map<std::string, PlyWriter::Callback> callbacks;
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

      std::visit(
          overloaded{
              [&](const std::span<const int8_t>& data) {
                callbacks.emplace(
                    property.first,
                    Int8PropertyCallback(&InMemoryWriter::Callback<int8_t>));
              },
              [&](const std::span<const std::span<const int8_t>>& data) {
                callbacks.emplace(property.first,
                                  Int8PropertyListCallback(
                                      &InMemoryWriter::ListCallback<int8_t>));
              },
              [&](const std::span<const uint8_t>& data) {
                callbacks.emplace(
                    property.first,
                    UInt8PropertyCallback(&InMemoryWriter::Callback<uint8_t>));
              },
              [&](const std::span<const std::span<const uint8_t>>& data) {
                callbacks.emplace(property.first,
                                  UInt8PropertyListCallback(
                                      &InMemoryWriter::ListCallback<uint8_t>));
              },
              [&](const std::span<const int16_t>& data) {
                callbacks.emplace(
                    property.first,
                    Int16PropertyCallback(&InMemoryWriter::Callback<int16_t>));
              },
              [&](const std::span<const std::span<const int16_t>>& data) {
                callbacks.emplace(property.first,
                                  Int16PropertyListCallback(
                                      &InMemoryWriter::ListCallback<int16_t>));
              },
              [&](const std::span<const uint16_t>& data) {
                callbacks.emplace(property.first,
                                  UInt16PropertyCallback(
                                      &InMemoryWriter::Callback<uint16_t>));
              },
              [&](const std::span<const std::span<const uint16_t>>& data) {
                callbacks.emplace(property.first,
                                  UInt16PropertyListCallback(
                                      &InMemoryWriter::ListCallback<uint16_t>));
              },
              [&](const std::span<const int32_t>& data) {
                callbacks.emplace(
                    property.first,
                    Int32PropertyCallback(&InMemoryWriter::Callback<int32_t>));
              },
              [&](const std::span<const std::span<const int32_t>>& data) {
                callbacks.emplace(property.first,
                                  Int32PropertyListCallback(
                                      &InMemoryWriter::ListCallback<int32_t>));
              },
              [&](const std::span<const uint32_t>& data) {
                callbacks.emplace(property.first,
                                  UInt32PropertyCallback(
                                      &InMemoryWriter::Callback<uint32_t>));
              },
              [&](const std::span<const std::span<const uint32_t>>& data) {
                callbacks.emplace(property.first,
                                  UInt32PropertyListCallback(
                                      &InMemoryWriter::ListCallback<uint32_t>));
              },
              [&](const std::span<const float>& data) {
                callbacks.emplace(
                    property.first,
                    FloatPropertyCallback(&InMemoryWriter::Callback<float>));
              },
              [&](const std::span<const std::span<const float>>& data) {
                callbacks.emplace(property.first,
                                  FloatPropertyListCallback(
                                      &InMemoryWriter::ListCallback<float>));
              },
              [&](const std::span<const double>& data) {
                callbacks.emplace(
                    property.first,
                    DoublePropertyCallback(&InMemoryWriter::Callback<double>));
              },
              [&](const std::span<const std::span<const double>>& data) {
                callbacks.emplace(property.first,
                                  DoublePropertyListCallback(
                                      &InMemoryWriter::ListCallback<double>));
              }},
          property.second);
    }

    property_callbacks[element.first] =
        std::make_pair(num_elements.value_or(0), std::move(callbacks));
  }

  comments.insert(comments.end(), comments_.begin(), comments_.end());
  object_info.insert(object_info.end(), object_info_.begin(),
                     object_info_.end());

  return std::expected<void, std::string>();
}

std::expected<PlyWriter::ListSizeType, std::string>
InMemoryWriter::GetPropertyListSizeType(const std::string& element_name,
                                        size_t element_index,
                                        const std::string& property_name,
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
      *indexed_properties_.at(element_index).second.at(property_index));

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

std::expected<void, std::string> WriteTo(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteTo(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToASCII(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToASCII(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToBigEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToBigEndian(stream);
}

// Most clients should prefer WriteTo over this
std::expected<void, std::string> WriteToLittleEndian(
    std::ostream& stream,
    const std::map<std::string, std::map<std::string, Property>>& properties,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
  InMemoryWriter writer(properties, comments, object_info);
  return writer.WriteToLittleEndian(stream);
}

}  // namespace plyodine
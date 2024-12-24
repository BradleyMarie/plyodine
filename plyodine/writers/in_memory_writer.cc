#include "plyodine/writers/in_memory_writer.h"

#include <any>
#include <cstdint>
#include <expected>
#include <limits>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

namespace plyodine {
namespace {

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override {
    return "plyodine::InMemoryWriter";
  }

  std::string message(int condition) const override {
    InMemoryWriter::ErrorCodes error_code{condition};
    switch (error_code) {
      case InMemoryWriter::ErrorCodes::UNBALANCED_PROPERTIES:
        return "All properties of an element must have the same size";
      case InMemoryWriter::ErrorCodes::PROPERTY_LIST_TOO_LONG:
        return "Property lists can contain no more than 4294967295 entries";
    }

    return "Unknown Error";
  }
} kErrorCategory;

}  // namespace

void InMemoryWriter::AddComment(std::string comment) {
  comments_.push_back(std::move(comment));
}

void InMemoryWriter::AddObjectInfo(std::string object_info) {
  object_info_.push_back(std::move(object_info));
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const int8_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const uint8_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const int16_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const uint16_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const int32_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const uint32_t> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const float> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyShallow(const std::string& element_name,
                                        const std::string& property_name,
                                        std::span<const double> values) {
  AddPropertyShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const int8_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const uint8_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const int16_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const uint16_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const int32_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const uint32_t> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const float> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::span<const double> values) {
  AddPropertyImpl(element_name, property_name, values);
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<int8_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<uint8_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<int16_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<uint16_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<int32_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<uint32_t>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<float>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddProperty(const std::string& element_name,
                                 const std::string& property_name,
                                 std::vector<double>&& values) {
  AddPropertyImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int8_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint8_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int16_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint16_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int32_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint32_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const float>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const double>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int8_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint8_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int16_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint16_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int32_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint32_t>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<float>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyListShallow(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<double>> values) {
  AddPropertyListShallowImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int8_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint8_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int16_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint16_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const int32_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const uint32_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const float>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::span<const double>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int8_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint8_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int16_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint16_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<int32_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<uint32_t>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<float>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::span<const std::vector<double>> values) {
  AddPropertyListImpl(element_name, property_name, values);
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<int8_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<uint8_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<int16_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<uint16_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<int32_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<uint32_t>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(const std::string& element_name,
                                     const std::string& property_name,
                                     std::vector<std::vector<float>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

void InMemoryWriter::AddPropertyList(
    const std::string& element_name, const std::string& property_name,
    std::vector<std::vector<double>>&& values) {
  AddPropertyListImpl(element_name, property_name, std::move(values));
}

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

std::error_code InMemoryWriter::Start(
    std::map<std::string, uintmax_t>& num_element_instances,
    std::map<std::string, std::map<std::string, Callback>>& callbacks,
    std::vector<std::string>& comments,
    std::vector<std::string>& object_info) const {
  for (const auto& element : properties_) {
    std::map<std::string, PlyWriter::Callback> property_callbacks;
    std::optional<size_t> num_elements;
    for (const auto& property : element.second) {
      size_t property_num_elements = std::visit(
          [](const auto& entry) { return entry.size(); }, property.second);

      if (num_elements.has_value()) {
        if (*num_elements != property_num_elements) {
          return std::error_code(
              static_cast<int>(ErrorCodes::UNBALANCED_PROPERTIES),
              kErrorCategory);
        }
      } else {
        num_elements = property_num_elements;
      }

      std::visit(
          overloaded{
              [&](const std::span<const int8_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int8PropertyCallback(
                        &InMemoryWriter::ScalarCallback<int8_t>));
              },
              [&](const std::span<const std::span<const int8_t>>& data) {
                property_callbacks.emplace(
                    property.first, Int8PropertyListCallback(
                                        &InMemoryWriter::SpanCallback<int8_t>));
              },
              [&](const std::span<const std::vector<int8_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int8PropertyListCallback(
                        &InMemoryWriter::VectorCallback<int8_t>));
              },
              [&](const std::span<const uint8_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt8PropertyCallback(
                        &InMemoryWriter::ScalarCallback<uint8_t>));
              },
              [&](const std::span<const std::span<const uint8_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt8PropertyListCallback(
                        &InMemoryWriter::SpanCallback<uint8_t>));
              },
              [&](const std::span<const std::vector<uint8_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt8PropertyListCallback(
                        &InMemoryWriter::VectorCallback<uint8_t>));
              },
              [&](const std::span<const int16_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int16PropertyCallback(
                        &InMemoryWriter::ScalarCallback<int16_t>));
              },
              [&](const std::span<const std::span<const int16_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int16PropertyListCallback(
                        &InMemoryWriter::SpanCallback<int16_t>));
              },
              [&](const std::span<const std::vector<int16_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int16PropertyListCallback(
                        &InMemoryWriter::VectorCallback<int16_t>));
              },
              [&](const std::span<const uint16_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt16PropertyCallback(
                        &InMemoryWriter::ScalarCallback<uint16_t>));
              },
              [&](const std::span<const std::span<const uint16_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt16PropertyListCallback(
                        &InMemoryWriter::SpanCallback<uint16_t>));
              },
              [&](const std::span<const std::vector<uint16_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt16PropertyListCallback(
                        &InMemoryWriter::VectorCallback<uint16_t>));
              },
              [&](const std::span<const int32_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int32PropertyCallback(
                        &InMemoryWriter::ScalarCallback<int32_t>));
              },
              [&](const std::span<const std::span<const int32_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int32PropertyListCallback(
                        &InMemoryWriter::SpanCallback<int32_t>));
              },
              [&](const std::span<const std::vector<int32_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    Int32PropertyListCallback(
                        &InMemoryWriter::VectorCallback<int32_t>));
              },
              [&](const std::span<const uint32_t>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt32PropertyCallback(
                        &InMemoryWriter::ScalarCallback<uint32_t>));
              },
              [&](const std::span<const std::span<const uint32_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt32PropertyListCallback(
                        &InMemoryWriter::SpanCallback<uint32_t>));
              },
              [&](const std::span<const std::vector<uint32_t>>& data) {
                property_callbacks.emplace(
                    property.first,
                    UInt32PropertyListCallback(
                        &InMemoryWriter::VectorCallback<uint32_t>));
              },
              [&](const std::span<const float>& data) {
                property_callbacks.emplace(
                    property.first,
                    FloatPropertyCallback(
                        &InMemoryWriter::ScalarCallback<float>));
              },
              [&](const std::span<const std::span<const float>>& data) {
                property_callbacks.emplace(
                    property.first, FloatPropertyListCallback(
                                        &InMemoryWriter::SpanCallback<float>));
              },
              [&](const std::span<const std::vector<float>>& data) {
                property_callbacks.emplace(
                    property.first,
                    FloatPropertyListCallback(
                        &InMemoryWriter::VectorCallback<float>));
              },
              [&](const std::span<const double>& data) {
                property_callbacks.emplace(
                    property.first,
                    DoublePropertyCallback(
                        &InMemoryWriter::ScalarCallback<double>));
              },
              [&](const std::span<const std::span<const double>>& data) {
                property_callbacks.emplace(
                    property.first, DoublePropertyListCallback(
                                        &InMemoryWriter::SpanCallback<double>));
              },
              [&](const std::span<const std::vector<double>>& data) {
                property_callbacks.emplace(
                    property.first,
                    DoublePropertyListCallback(
                        &InMemoryWriter::VectorCallback<double>));
              }},
          property.second);
    }

    callbacks[element.first] = std::move(property_callbacks);
    num_element_instances[element.first] = num_elements.value_or(0);
  }

  comments.insert(comments.end(), comments_.begin(), comments_.end());
  object_info.insert(object_info.end(), object_info_.begin(),
                     object_info_.end());

  return std::error_code();
}

std::expected<PlyWriter::ListSizeType, std::error_code>
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

  return std::unexpected(std::error_code(
      static_cast<int>(ErrorCodes::PROPERTY_LIST_TOO_LONG), kErrorCategory));
}

}  // namespace plyodine
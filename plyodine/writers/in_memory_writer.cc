#include "plyodine/writers/in_memory_writer.h"

#include <any>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace {

std::generator<int8_t> MakeGenerator(std::span<const int8_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int8_t>> MakeGenerator(
    std::span<const std::span<const int8_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int8_t>> MakeGenerator(
    std::span<const std::vector<int8_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<uint8_t> MakeGenerator(std::span<const uint8_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint8_t>> MakeGenerator(
    std::span<const std::span<const uint8_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint8_t>> MakeGenerator(
    std::span<const std::vector<uint8_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<int16_t> MakeGenerator(std::span<const int16_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int16_t>> MakeGenerator(
    std::span<const std::span<const int16_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int16_t>> MakeGenerator(
    std::span<const std::vector<int16_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<uint16_t> MakeGenerator(std::span<const uint16_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint16_t>> MakeGenerator(
    std::span<const std::span<const uint16_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint16_t>> MakeGenerator(
    std::span<const std::vector<uint16_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<int32_t> MakeGenerator(std::span<const int32_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int32_t>> MakeGenerator(
    std::span<const std::span<const int32_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const int32_t>> MakeGenerator(
    std::span<const std::vector<int32_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<uint32_t> MakeGenerator(std::span<const uint32_t> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint32_t>> MakeGenerator(
    std::span<const std::span<const uint32_t>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const uint32_t>> MakeGenerator(
    std::span<const std::vector<uint32_t>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<float> MakeGenerator(std::span<const float> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const float>> MakeGenerator(
    std::span<const std::span<const float>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const float>> MakeGenerator(
    std::span<const std::vector<float>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

std::generator<double> MakeGenerator(std::span<const double> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const double>> MakeGenerator(
    std::span<const std::span<const double>> values) {
  for (const auto& value : values) {
    co_yield value;
  }
}

std::generator<std::span<const double>> MakeGenerator(
    std::span<const std::vector<double>> values) {
  for (const auto& value : values) {
    co_yield std::span(value);
  }
}

}  // namespace

namespace plyodine {

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

std::error_code InMemoryWriter::Start(
    std::map<std::string, uintmax_t>& num_element_instances,
    std::map<std::string, std::map<std::string, PropertyGenerator>>&
        property_generators,
    std::vector<std::string>& comments,
    std::vector<std::string>& object_info) const {
  comments.insert(comments.end(), comments_.begin(), comments_.end());
  object_info.insert(object_info.end(), object_info_.begin(),
                     object_info_.end());

  for (const auto& [element_name, element_properties] : properties_) {
    std::map<std::string, PlyWriter::PropertyGenerator>& generators =
        property_generators[element_name];
    size_t& num_elements = num_element_instances[element_name];
    for (const auto& [property_name, property] : element_properties) {
      generators.try_emplace(property_name,
                             std::visit(
                                 [&](const auto& values) -> PropertyGenerator {
                                   return MakeGenerator(values);
                                 },
                                 property));
      num_elements = std::max(
          num_elements,
          std::visit([](const auto& list) { return list.size(); }, property));
    }
  }

  return std::error_code();
}

PlyWriter::ListSizeType InMemoryWriter::GetPropertyListSizeType(
    const std::string& element_name, const std::string& property_name) const {
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
      properties_.find(element_name)->second.find(property_name)->second);

  if (max_size <= std::numeric_limits<uint8_t>::max()) {
    return PlyWriter::ListSizeType::UCHAR;
  }

  if (max_size <= std::numeric_limits<uint16_t>::max()) {
    return PlyWriter::ListSizeType::USHORT;
  }

  return PlyWriter::ListSizeType::UINT;
}

}  // namespace plyodine
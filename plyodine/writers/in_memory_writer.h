#ifndef _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#define _PLYODINE_WRITERS_IN_MEMORY_WRITER_

#include <any>
#include <cstdint>
#include <expected>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_writer.h"

namespace plyodine {

class InMemoryWriter final : public PlyWriter {
 public:
  void AddComment(std::string comment);

  void AddObjectInfo(std::string object_info);

  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int8_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint8_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int16_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint16_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int32_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint32_t> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const float> values);
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const double> values);

  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int8_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint8_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int16_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint16_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int32_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint32_t> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const float> values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const double> values);

  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int8_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint8_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int16_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint16_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int32_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint32_t>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<float>&& values);
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<double>&& values);

  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int8_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const uint8_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int16_t>> values);
  void AddPropertyListShallow(
      const std::string& element_name, const std::string& property_name,
      std::span<const std::span<const uint16_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int32_t>> values);
  void AddPropertyListShallow(
      const std::string& element_name, const std::string& property_name,
      std::span<const std::span<const uint32_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const float>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const double>> values);

  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int8_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint8_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int16_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint16_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int32_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint32_t>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<float>> values);
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<double>> values);

  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int8_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint8_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int16_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint16_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int32_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint32_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const float>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const double>> values);

  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int8_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint8_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int16_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint16_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int32_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint32_t>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<float>> values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<double>> values);

  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int8_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint8_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int16_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint16_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int32_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint32_t>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<float>>&& values);
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<double>>&& values);

 protected:
  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, Callback>>& callbacks,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override;

  std::expected<PlyWriter::ListSizeType, std::error_code>
  GetPropertyListSizeType(const std::string& element_name, size_t element_index,
                          const std::string& property_name,
                          size_t property_index) const override;

 private:
  void ReindexProperties() {
    indexed_properties_.clear();
    for (const auto& element : properties_) {
      indexed_properties_.emplace_back();
      for (const auto& property : element.second) {
        indexed_properties_.back().emplace_back(&property.second);
      }
    }
  }

  template <typename T>
  void AddPropertyShallowImpl(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const T> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name].reset();

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyImpl(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const T> values) {
    property_storage_[element_name][property_name] =
        std::vector<T>(values.begin(), values.end());
    properties_[element_name][property_name] = std::any_cast<std::vector<T>&>(
        property_storage_[element_name][property_name]);

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyImpl(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<T>&& values) {
    property_storage_[element_name][property_name] = std::move(values);
    properties_[element_name][property_name] = std::any_cast<std::vector<T>&>(
        property_storage_[element_name][property_name]);

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyListShallowImpl(const std::string& element_name,
                                  const std::string& property_name,
                                  std::span<const std::span<const T>> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name].reset();

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyListShallowImpl(const std::string& element_name,
                                  const std::string& property_name,
                                  std::span<const std::vector<T>> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name].reset();

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::span<const std::span<const T>> values) {
    std::vector<std::vector<T>> copy;
    for (const auto& value : values) {
      copy.emplace_back(value.begin(), value.end());
    }

    property_storage_[element_name][property_name] = std::move(copy);
    properties_[element_name][property_name] =
        std::any_cast<std::vector<std::vector<T>>&>(
            property_storage_[element_name][property_name]);

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::span<const std::vector<T>> values) {
    std::vector<std::vector<T>> copy;
    for (const auto& value : values) {
      copy.emplace_back(value.begin(), value.end());
    }

    property_storage_[element_name][property_name] = std::move(copy);
    properties_[element_name][property_name] =
        std::any_cast<std::vector<std::vector<T>>&>(
            property_storage_[element_name][property_name]);

    ReindexProperties();
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::vector<std::vector<T>>&& values) {
    properties_[element_name][property_name] = std::move(values);
    property_storage_[element_name][property_name].reset();

    ReindexProperties();
  }

  template <typename T>
  std::expected<T, std::error_code> ScalarCallback(
      const std::string& element_name, size_t element_index,
      const std::string& property_name, size_t property_index,
      uintmax_t instance) const {
    return std::get<std::span<const T>>(
        *indexed_properties_[element_index]
                            [property_index])[static_cast<size_t>(instance)];
  }

  template <typename T>
  std::expected<std::span<const T>, std::error_code> SpanCallback(
      const std::string& element_name, size_t element_index,
      const std::string& property_name, size_t property_index,
      uintmax_t instance, std::vector<T>& storage) const {
    return std::get<std::span<const std::span<const T>>>(
        *indexed_properties_[element_index]
                            [property_index])[static_cast<size_t>(instance)];
  }

  template <typename T>
  std::expected<std::span<const T>, std::error_code> VectorCallback(
      const std::string& element_name, size_t element_index,
      const std::string& property_name, size_t property_index,
      uintmax_t instance, std::vector<T>& storage) const {
    return std::get<std::span<const std::vector<T>>>(
        *indexed_properties_[element_index]
                            [property_index])[static_cast<size_t>(instance)];
  }

  typedef std::variant<
      std::span<const int8_t>, std::span<const std::span<const int8_t>>,
      std::span<const std::vector<int8_t>>, std::span<const uint8_t>,
      std::span<const std::span<const uint8_t>>,
      std::span<const std::vector<uint8_t>>, std::span<const int16_t>,
      std::span<const std::span<const int16_t>>,
      std::span<const std::vector<int16_t>>, std::span<const uint16_t>,
      std::span<const std::span<const uint16_t>>,
      std::span<const std::vector<uint16_t>>, std::span<const int32_t>,
      std::span<const std::span<const int32_t>>,
      std::span<const std::vector<int32_t>>, std::span<const uint32_t>,
      std::span<const std::span<const uint32_t>>,
      std::span<const std::vector<uint32_t>>, std::span<const float>,
      std::span<const std::span<const float>>,
      std::span<const std::vector<float>>, std::span<const double>,
      std::span<const std::span<const double>>,
      std::span<const std::vector<double>>>
      Property;

  std::vector<std::string> comments_;
  std::vector<std::string> object_info_;
  std::map<std::string, std::map<std::string, Property>> properties_;
  std::map<std::string, std::map<std::string, std::any>> property_storage_;
  std::vector<std::vector<const Property*>> indexed_properties_;
};

}  // namespace plyodine

#endif  // _PLYODINE_WRITERS_IN_MEMORY_WRITER_
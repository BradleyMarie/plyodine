#ifndef _PLYODINE_WRITERS_IN_MEMORY_WRITER_
#define _PLYODINE_WRITERS_IN_MEMORY_WRITER_

#include <cstdint>
#include <generator>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

#include "plyodine/ply_writer.h"

namespace plyodine {

// A PLY writer that works with values that are fully present in memory.
//
// NOTE: The interface of this class is not yet fully stable and as such this
// class should be considered experimental. It is possible that breaking changes
// may be made to this class in the future which will not be reflected in the
// plyodine major version number.
class InMemoryWriter final : public PlyWriter {
 public:
  // Add a comment to the file
  void AddComment(std::string comment);

  // Adds an object info to the the file
  void AddObjectInfo(std::string object_info);

  // Add a char property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int8_t> values);

  // Add a uchar property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint8_t> values);

  // Add a short property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int16_t> values);

  // Add a ushort property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint16_t> values);

  // Add an int property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const int32_t> values);

  // Add a uint property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const uint32_t> values);

  // Add a float property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const float> values);

  // Add a double property to the file without copying or moving the values into
  // this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyShallow(const std::string& element_name,
                          const std::string& property_name,
                          std::span<const double> values);

  // Add a char property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int8_t> values);

  // Add a uchar property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint8_t> values);

  // Add a short property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int16_t> values);

  // Add a ushort property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint16_t> values);

  // Add an int property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const int32_t> values);

  // Add a uint property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const uint32_t> values);

  // Add a float property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const float> values);

  // Add a double property to the file by copying the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::span<const double> values);

  // Add a char property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int8_t>&& values);

  // Add a uchar property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint8_t>&& values);

  // Add a short property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int16_t>&& values);

  // Add a ushort property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint16_t>&& values);

  // Add an int property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<int32_t>&& values);

  // Add a uint property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<uint32_t>&& values);

  // Add a float property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<float>&& values);

  // Add a double property to the file by moving the values into this object.
  void AddProperty(const std::string& element_name,
                   const std::string& property_name,
                   std::vector<double>&& values);

  // Add a char property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int8_t>> values);

  // Add a uchar property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const uint8_t>> values);

  // Add a short property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int16_t>> values);

  // Add a ushort property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(
      const std::string& element_name, const std::string& property_name,
      std::span<const std::span<const uint16_t>> values);

  // Add an int property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const int32_t>> values);

  // Add a uint property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(
      const std::string& element_name, const std::string& property_name,
      std::span<const std::span<const uint32_t>> values);

  // Add a float property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const float>> values);

  // Add a double property list to the file without copying or moving the values
  // into this object.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::span<const double>> values);

  // Add a char property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int8_t>> values);

  // Add a uchar property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint8_t>> values);

  // Add a short property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int16_t>> values);

  // Add a ushort property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint16_t>> values);

  // Add an int property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<int32_t>> values);

  // Add a uint property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<uint32_t>> values);

  // Add a float property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<float>> values);

  // Add a double property list to the file without copying or moving the values
  // of the property list list.
  //
  // It is up to the caller to ensure that the lifetime of the data in `values`
  // exceeds the lifetime of this object.
  void AddPropertyListShallow(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const std::vector<double>> values);

  // Add a char property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int8_t>> values);

  // Add a uchar property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint8_t>> values);

  // Add a short property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int16_t>> values);

  // Add a ushort property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint16_t>> values);

  // Add an int property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const int32_t>> values);

  // Add a uint property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const uint32_t>> values);

  // Add a float property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const float>> values);

  // Add a double property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::span<const double>> values);

  // Add a char property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int8_t>> values);

  // Add a uchar property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint8_t>> values);

  // Add a short property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int16_t>> values);

  // Add a ushort property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint16_t>> values);

  // Add an int property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<int32_t>> values);

  // Add a uint property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<uint32_t>> values);

  // Add a float property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<float>> values);

  // Add a double property list to the file by copying the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const std::vector<double>> values);

  // Add a char property list to the file by moving the values into this object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int8_t>>&& values);

  // Add a uchar property list to the file by moving the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint8_t>>&& values);

  // Add a short property list to the file by moving the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int16_t>>&& values);

  // Add a ushort property list to the file by moving the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint16_t>>&& values);

  // Add an int property list to the file by moving the values into this object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<int32_t>>&& values);

  // Add a uint property list to the file by moving the values into this object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<uint32_t>>&& values);

  // Add a float property list to the file by moving the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<float>>&& values);

  // Add a double property list to the file by moving the values into this
  // object.
  void AddPropertyList(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<std::vector<double>>&& values);

 protected:
  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          property_generators,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override;

  PlyWriter::ListSizeType GetPropertyListSizeType(
      const std::string& element_name,
      const std::string& property_name) const override;

 private:
  template <typename T>
  void AddPropertyShallowImpl(const std::string& element_name,
                              const std::string& property_name,
                              std::span<const T> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name] = PropertyStorage();
  }

  template <typename T>
  void AddPropertyImpl(const std::string& element_name,
                       const std::string& property_name,
                       std::span<const T> values) {
    properties_[element_name][property_name] =
        property_storage_[element_name][property_name].emplace<std::vector<T>>(
            values.begin(), values.end());
  }

  template <typename T>
  void AddPropertyImpl(const std::string& element_name,
                       const std::string& property_name,
                       std::vector<T>&& values) {
    properties_[element_name][property_name] =
        property_storage_[element_name][property_name].emplace<std::vector<T>>(
            std::move(values));
  }

  template <typename T>
  void AddPropertyListShallowImpl(const std::string& element_name,
                                  const std::string& property_name,
                                  std::span<const std::span<const T>> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name] = PropertyStorage();
  }

  template <typename T>
  void AddPropertyListShallowImpl(const std::string& element_name,
                                  const std::string& property_name,
                                  std::span<const std::vector<T>> values) {
    properties_[element_name][property_name] = values;
    property_storage_[element_name][property_name] = PropertyStorage();
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::span<const std::span<const T>> values) {
    std::vector<std::vector<T>>& copy =
        property_storage_[element_name][property_name]
            .emplace<std::vector<std::vector<T>>>();
    for (const auto& value : values) {
      copy.emplace_back(value.begin(), value.end());
    }

    properties_[element_name][property_name] = copy;
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::span<const std::vector<T>> values) {
    std::vector<std::vector<T>>& copy =
        property_storage_[element_name][property_name]
            .emplace<std::vector<std::vector<T>>>();
    for (const auto& value : values) {
      copy.emplace_back(value.begin(), value.end());
    }

    properties_[element_name][property_name] = copy;
  }

  template <typename T>
  void AddPropertyListImpl(const std::string& element_name,
                           const std::string& property_name,
                           std::vector<std::vector<T>>&& values) {
    properties_[element_name][property_name] = std::move(values);
    property_storage_[element_name][property_name] = PropertyStorage();
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

  typedef std::variant<std::monostate, std::vector<int8_t>,
                       std::vector<std::vector<int8_t>>, std::vector<uint8_t>,
                       std::vector<std::vector<uint8_t>>, std::vector<int16_t>,
                       std::vector<std::vector<int16_t>>, std::vector<uint16_t>,
                       std::vector<std::vector<uint16_t>>, std::vector<int32_t>,
                       std::vector<std::vector<int32_t>>, std::vector<uint32_t>,
                       std::vector<std::vector<uint32_t>>, std::vector<float>,
                       std::vector<std::vector<float>>, std::vector<double>,
                       std::vector<std::vector<double>>>
      PropertyStorage;

  std::vector<std::string> comments_;
  std::vector<std::string> object_info_;
  std::map<std::string, std::map<std::string, Property>> properties_;
  std::map<std::string, std::map<std::string, PropertyStorage>>
      property_storage_;
};

}  // namespace plyodine

#endif  // _PLYODINE_WRITERS_IN_MEMORY_WRITER_
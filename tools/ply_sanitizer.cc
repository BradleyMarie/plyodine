#include <cstdint>
#include <expected>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

#include "plyodine/ply_header_reader.h"
#include "plyodine/ply_reader.h"
#include "plyodine/ply_writer.h"

namespace plyodine {
namespace {

class Sanitizer : private PlyReader, private PlyWriter {
 public:
  std::error_code Sanitize(std::istream& input, std::ostream& output);

 private:
  struct PropertyInterface {
    virtual ~PropertyInterface() = default;
    virtual PropertyGenerator GetGenerator() const = 0;
  };

  template <typename T>
  class Property : public PropertyInterface {
   public:
    void Add(const T& value) { values_.push_back(value); }

    PropertyGenerator GetGenerator() const { return MakeGenerator(); }

   private:
    std::generator<T> MakeGenerator() const {
      for (auto value : values_) {
        co_yield value;
      }
    }

    std::vector<T> values_;
  };

  template <typename T>
  struct PropertyList : public PropertyInterface {
   public:
    void Add(std::span<const T> value) {
      values_.emplace_back(value.begin(), value.end());
    }

    PropertyGenerator GetGenerator() const { return MakeGenerator(); }

   private:
    std::generator<std::span<const T>> MakeGenerator() const {
      for (auto value : values_) {
        co_yield value;
      }
    }

    std::vector<std::vector<T>> values_;
  };

  template <typename T>
  static std::unique_ptr<PropertyInterface> UpdateCallback(
      std::function<std::error_code(T)>& callback);

  template <typename T>
  static std::unique_ptr<PropertyInterface> UpdateCallback(
      std::function<std::error_code(std::span<const T>)>& callback);

  // PlyReader
  std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) override;

  // PlyWriter
  std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          property_generators,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const override;

  ListSizeType GetPropertyListSizeType(
      const std::string& element_name,
      const std::string& property_name) const override;

  std::map<std::string, uintmax_t> num_element_instances_;
  std::map<std::string,
           std::map<std::string, std::unique_ptr<PropertyInterface>>>
      elements_;
  std::map<std::string, std::map<std::string, ListSizeType>> list_size_types_;
  std::vector<std::string> comments_;
  std::vector<std::string> object_info_;
};

std::error_code Sanitizer::Sanitize(std::istream& input, std::ostream& output) {
  num_element_instances_.clear();
  elements_.clear();
  list_size_types_.clear();
  comments_.clear();
  object_info_.clear();

  auto header = ReadPlyHeader(input);
  if (!header) {
    return header.error();
  }

  for (const auto& element : header->elements) {
    for (const auto& property : element.properties) {
      if (!property.list_type) {
        continue;
      }

      switch (*property.list_type) {
        case PlyHeader::Property::Type::CHAR:
        case PlyHeader::Property::Type::UCHAR:
          list_size_types_[element.name][property.name] = ListSizeType::UCHAR;
          break;
        case PlyHeader::Property::Type::SHORT:
        case PlyHeader::Property::Type::USHORT:
          list_size_types_[element.name][property.name] = ListSizeType::USHORT;
          break;
        case PlyHeader::Property::Type::INT:
        case PlyHeader::Property::Type::UINT:
          list_size_types_[element.name][property.name] = ListSizeType::UINT;
          break;
        default:
          break;
      }
    }
  }

  input.seekg(0);

  if (std::error_code error = ReadFrom(input); error) {
    return error;
  }

  return WriteTo(output);
}

template <typename T>
std::unique_ptr<Sanitizer::PropertyInterface> Sanitizer::UpdateCallback(
    std::function<std::error_code(T)>& callback) {
  std::unique_ptr<Property<T>> property = std::make_unique<Property<T>>();
  callback = [ptr = property.get()](T value) -> std::error_code {
    ptr->Add(value);
    return std::error_code();
  };
  return property;
}

template <typename T>
std::unique_ptr<Sanitizer::PropertyInterface> Sanitizer::UpdateCallback(
    std::function<std::error_code(std::span<const T>)>& callback) {
  std::unique_ptr<PropertyList<T>> property_list =
      std::make_unique<PropertyList<T>>();
  callback = [ptr = property_list.get()](
                 std::span<const T> values) -> std::error_code {
    ptr->Add(values);
    return std::error_code();
  };
  return property_list;
}

// PlyReader
std::error_code Sanitizer::Start(
    std::map<std::string, uintmax_t> num_element_instances,
    std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
    std::vector<std::string> comments, std::vector<std::string> object_info) {
  num_element_instances_ = std::move(num_element_instances);
  comments_ = std::move(comments);
  object_info_ = std::move(object_info);

  for (auto& [element_name, element] : callbacks) {
    for (auto& [property_name, property_callback] : element) {
      elements_[element_name][property_name] = std::visit(
          [](auto& callback) -> std::unique_ptr<PropertyInterface> {
            return UpdateCallback(callback);
          },
          property_callback);
    }
  }

  return std::error_code();
}

// PlyWriter
std::error_code Sanitizer::Start(
    std::map<std::string, uintmax_t>& num_element_instances,
    std::map<std::string, std::map<std::string, PropertyGenerator>>&
        property_generators,
    std::vector<std::string>& comments,
    std::vector<std::string>& object_info) const {
  num_element_instances = std::move(num_element_instances_);
  comments = std::move(comments_);
  object_info = std::move(object_info_);

  for (const auto& [element_name, element] : elements_) {
    for (const auto& [property_name, property] : element) {
      property_generators[element_name].try_emplace(property_name,
                                                    property->GetGenerator());
    }
  }

  return std::error_code();
}

PlyWriter::ListSizeType Sanitizer::GetPropertyListSizeType(
    const std::string& element_name, const std::string& property_name) const {
  return list_size_types_.find(element_name)
      ->second.find(property_name)
      ->second;
}

}  // namespace
}  // namespace plyodine

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: ply_sanitizer <input> <output>" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream input(argv[1], std::ios_base::in | std::ios_base::binary);
  if (!input) {
    std::cerr << "failed to open input" << std::endl;
    return EXIT_FAILURE;
  }

  std::ofstream output(argv[2], std::ios_base::out | std::ios_base::binary);
  if (!output) {
    std::cerr << "failed to open output" << std::endl;
    return EXIT_FAILURE;
  }

  plyodine::Sanitizer sanitizer;
  if (std::error_code error = sanitizer.Sanitize(input, output); error) {
    std::cerr << error.message() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
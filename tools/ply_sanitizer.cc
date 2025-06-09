#include <condition_variable>
#include <cstdint>
#include <expected>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_header_reader.h"
#include "plyodine/ply_reader.h"
#include "plyodine/ply_writer.h"

namespace plyodine {
namespace {

enum class Format { ASCII, BIG, LITTLE, NATIVE };

class Sanitizer final : private PlyReader, private PlyWriter {
 public:
  Sanitizer(bool low_mem) : low_mem_(low_mem) {}

  std::error_code Sanitize(std::optional<Format> format, std::istream& input,
                           std::ostream& output);

 private:
  struct PropertyInterface {
    virtual ~PropertyInterface() = default;
    virtual PropertyGenerator GetGenerator() = 0;
    virtual void Cancel() = 0;
  };

  template <typename Storage, typename View>
  class Property : public PropertyInterface {
   public:
    Property(bool low_mem, uintmax_t num_instances)
        : low_mem_(low_mem), num_instances_(num_instances) {}

    PropertyGenerator GetGenerator() override { return MakeGenerator(); }

    void Cancel() override {
      if (low_mem_) {
        std::unique_lock lock(mutex_);
        cancelled_ = true;
        condition_.notify_all();
      }
    }

    std::error_code Add(const View& value) {
      if (low_mem_) {
        std::unique_lock lock(mutex_);
        condition_.wait(lock,
                        [this]() { return values_.empty() || cancelled_; });
        if (cancelled_) {
          return std::make_error_code(std::errc::operation_canceled);
        }

        if constexpr (std::is_arithmetic_v<View>) {
          values_.push_back(value);
        } else {
          values_.emplace_back(value.begin(), value.end());
        }

        condition_.notify_all();
      } else {
        if constexpr (std::is_arithmetic_v<View>) {
          values_.push_back(value);
        } else {
          values_.emplace_back(value.begin(), value.end());
        }
      }

      return std::error_code();
    }

   private:
    std::generator<View> MakeGenerator() {
      std::vector<Storage> storage;
      while (num_instances_ != 0) {
        if (!Next(storage)) {
          break;
        }

        for (const auto& entry : storage) {
          num_instances_ -= 1;
          co_yield entry;
        }
      }
    }

    bool Next(std::vector<Storage>& result) {
      if (low_mem_) {
        std::unique_lock lock(mutex_);
        condition_.wait(lock,
                        [this]() { return !values_.empty() || cancelled_; });

        if (cancelled_) {
          return false;
        }

        std::swap(result, values_);
        values_.clear();

        condition_.notify_all();
      } else {
        std::swap(result, values_);
        values_.clear();
      }

      return true;
    }

    const bool low_mem_;
    uintmax_t num_instances_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<Storage> values_;
    bool cancelled_ = false;
  };

  template <typename T>
  static std::unique_ptr<PropertyInterface> UpdateCallback(
      std::function<std::error_code(T)>& callback, bool low_mem,
      uintmax_t num_instances);

  template <typename T>
  static std::unique_ptr<PropertyInterface> UpdateCallback(
      std::function<std::error_code(std::span<const T>)>& callback,
      bool low_mem, uintmax_t num_instances);

  void Cancel();

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

  size_t GetElementRank(const std::string& element_name) const override;

  size_t GetPropertyRank(const std::string& element_name,
                         const std::string& property_name) const override;

  const bool low_mem_;
  std::map<std::string, uintmax_t> num_element_instances_;
  std::map<std::string, size_t> element_rank;
  std::map<std::string, size_t> property_rank;
  std::map<std::string,
           std::map<std::string, std::unique_ptr<PropertyInterface>>>
      elements_;
  std::map<std::string, std::map<std::string, ListSizeType>> list_size_types_;
  std::vector<std::string> comments_;
  std::vector<std::string> object_info_;
  Format format_;

  // Writer State
  mutable std::future<std::error_code> write_result_;
  std::ostream* output_;
};

std::error_code Sanitizer::Sanitize(std::optional<Format> format,
                                    std::istream& input, std::ostream& output) {
  num_element_instances_.clear();
  elements_.clear();
  list_size_types_.clear();
  comments_.clear();
  object_info_.clear();
  element_rank.clear();
  property_rank.clear();

  auto header = ReadPlyHeader(input);
  if (!header) {
    return header.error();
  }

  comments_ = std::move(header->comments);
  object_info_ = std::move(header->object_info);

  if (!format.has_value()) {
    switch (header->format) {
      case PlyHeader::Format::ASCII:
        format_ = Format::ASCII;
        break;
      case PlyHeader::Format::BINARY_BIG_ENDIAN:
        format_ = Format::BIG;
        break;
      case PlyHeader::Format::BINARY_LITTLE_ENDIAN:
        format_ = Format::LITTLE;
        break;
    }
  } else {
    format_ = *format;
  }

  for (size_t i = 0; i < header->elements.size(); i++) {
    const auto& element = header->elements[i];
    num_element_instances_[element.name] = element.instance_count;
    element_rank[element.name] = i;

    for (size_t j = 0; j < element.properties.size(); j++) {
      const auto& property = element.properties[j];

      std::string key = element.name + " " + property.name;
      property_rank[property.name] = j;

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
  output_ = &output;

  if (std::error_code error = ReadFrom(input);
      error && error != std::errc::operation_canceled) {
    Cancel();
    return error;
  }

  return write_result_.get();
}

template <typename T>
std::unique_ptr<Sanitizer::PropertyInterface> Sanitizer::UpdateCallback(
    std::function<std::error_code(T)>& callback, bool low_mem,
    uintmax_t num_instances) {
  std::unique_ptr<Property<T, T>> property =
      std::make_unique<Property<T, T>>(low_mem, num_instances);
  callback = [ptr = property.get()](T value) -> std::error_code {
    return ptr->Add(value);
  };
  return property;
}

template <typename T>
std::unique_ptr<Sanitizer::PropertyInterface> Sanitizer::UpdateCallback(
    std::function<std::error_code(std::span<const T>)>& callback, bool low_mem,
    uintmax_t num_instances) {
  std::unique_ptr<Property<std::vector<T>, std::span<const T>>> property_list =
      std::make_unique<Property<std::vector<T>, std::span<const T>>>(
          low_mem, num_instances);
  callback = [ptr = property_list.get()](
                 std::span<const T> values) -> std::error_code {
    return ptr->Add(values);
  };
  return property_list;
}

void Sanitizer::Cancel() {
  if (!low_mem_) {
    return;
  }

  for (auto& element : elements_) {
    for (auto& [property_name, property] : element.second) {
      property->Cancel();
    }
  }
}

// PlyReader
std::error_code Sanitizer::Start(
    std::map<std::string, uintmax_t> num_element_instances,
    std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
    std::vector<std::string> comments, std::vector<std::string> object_info) {
  for (auto& [element_name, element] : callbacks) {
    for (auto& [property_name, property_callback] : element) {
      elements_[element_name][property_name] = std::visit(
          [&](auto& callback) -> std::unique_ptr<PropertyInterface> {
            return UpdateCallback(callback, low_mem_,
                                  num_element_instances[element_name]);
          },
          property_callback);
    }
  }

  std::launch launch_policy =
      low_mem_ ? std::launch::async : std::launch::deferred;

  write_result_ = std::async(launch_policy, [this]() {
    std::error_code error;
    switch (format_) {
      case Format::ASCII:
        error = WriteToASCII(*output_);
        break;
      case Format::BIG:
        error = WriteToBigEndian(*output_);
        break;
      case Format::LITTLE:
        error = WriteToLittleEndian(*output_);
        break;
      case Format::NATIVE:
        error = WriteTo(*output_);
        break;
    }

    if (error) {
      Cancel();
    }

    return error;
  });

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

size_t Sanitizer::GetElementRank(const std::string& element_name) const {
  return element_rank.find(element_name)->second;
}

size_t Sanitizer::GetPropertyRank(const std::string& element_name,
                                  const std::string& property_name) const {
  std::string key = element_name + " " + property_name;
  return property_rank.find(key)->second;
}

}  // namespace
}  // namespace plyodine

int main(int argc, char* argv[]) {
  static constexpr char usage[] =
      "usage: ply_sanitizer input output <[ascii|big|little|native]> "
      "<[lowmem]>";

  if (argc < 3 && argc > 5) {
    std::cerr << usage << std::endl;
    return EXIT_FAILURE;
  }

  bool lowmem = false;
  std::optional<plyodine::Format> format;
  if (argc == 4) {
    std::string_view arg = argv[3];
    if (arg == "ascii") {
      format = plyodine::Format::ASCII;
    } else if (arg == "big") {
      format = plyodine::Format::BIG;
    } else if (arg == "little") {
      format = plyodine::Format::LITTLE;
    } else if (arg == "native") {
      format = plyodine::Format::NATIVE;
    } else if (arg == "lowmem") {
      lowmem = true;
    } else {
      std::cerr << usage << std::endl;
      return EXIT_FAILURE;
    }
  } else if (argc == 5) {
    if (std::string_view(argv[4]) != "lowmem") {
      std::cerr << usage << std::endl;
      return EXIT_FAILURE;
    }

    std::string_view type = argv[3];
    if (type == "ascii") {
      format = plyodine::Format::ASCII;
    } else if (type == "big") {
      format = plyodine::Format::BIG;
    } else if (type == "little") {
      format = plyodine::Format::LITTLE;
    } else if (type == "native") {
      format = plyodine::Format::NATIVE;
    } else {
      std::cerr << usage << std::endl;
      return EXIT_FAILURE;
    }

    lowmem = true;
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

  plyodine::Sanitizer sanitizer(lowmem);
  if (std::error_code error = sanitizer.Sanitize(format, input, output);
      error) {
    std::cerr << error.message() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

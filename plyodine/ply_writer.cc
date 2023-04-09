#include "plyodine/ply_writer.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

namespace plyodine {
namespace {

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::stringstream>
    Context;

std::string FloatingPointError() {
  return "Only finite floating point values may be serialized to an ASCII "
         "output";
}

std::string ListSizeError() {
  return "The list was too big to be represented with the selected size type";
}

std::string WriteFailure() { return "Write failure"; }

std::expected<void, std::string> ValidateName(const std::string& name) {
  if (name.empty()) {
    return std::unexpected("Names of properties and elements may not be empty");
  }

  for (char c : name) {
    if (!std::isgraph(c)) {
      return std::unexpected(
          "Names of properties and elements may only contain graphic "
          "characters");
    }
  }

  return std::expected<void, std::string>();
}

bool ValidateComment(const std::string& comment) {
  for (char c : comment) {
    if (c == '\r' || c == '\n') {
      return false;
    }
  }

  return true;
}

template <std::integral SizeType, std::integral T>
std::expected<void, std::string> SerializeASCII(std::ostream& output,
                                                Context& context, T value) {
  output << +value;
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string>();
}

template <std::integral SizeType, std::floating_point T>
std::expected<void, std::string> SerializeASCII(std::ostream& output,
                                                Context& context, T value) {
  if (!std::isfinite(value)) {
    return std::unexpected(FloatingPointError());
  }

  std::stringstream& storage = std::get<std::stringstream>(context);

  storage.str("");

  int log = static_cast<int>(std::log10(std::abs(value))) + 1;
  int num_digits = std::max(std::numeric_limits<T>::max_digits10 - log, 0);
  storage << std::fixed << std::setprecision(num_digits) << value;

  std::string_view result = storage.view();

  size_t dot = result.find(".");
  if (dot != std::string_view::npos) {
    result = result.substr(0u, result.find_last_not_of("0") + 1u);
    if (result.back() == '.') {
      result.remove_suffix(1u);
    }
  }

  output << result;
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string>();
}

template <std::integral SizeType, typename T>
std::expected<void, std::string> SerializeASCII(std::ostream& output,
                                                Context& context,
                                                std::span<const T> values) {
  if (std::numeric_limits<SizeType>::max() < values.size()) {
    return std::unexpected(ListSizeError());
  }

  auto list_size_success = SerializeASCII<SizeType, SizeType>(
      output, context, static_cast<SizeType>(values.size()));
  if (!list_size_success) {
    return list_size_success;
  }

  for (const auto& entry : values) {
    output << ' ';
    if (!output) {
      return std::unexpected(WriteFailure());
    }

    auto entry_success = SerializeASCII<SizeType>(output, context, entry);
    if (!entry_success) {
      return entry_success;
    }
  }

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, std::integral T>
std::expected<void, std::string> SerializeBinary(std::ostream& output,
                                                 T value) {
  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  output.write(reinterpret_cast<char*>(&value), sizeof(value));
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, std::floating_point T>
std::expected<void, std::string> SerializeBinary(std::ostream& output,
                                                 T value) {
  auto entry = std::bit_cast<
      std::conditional_t<std::is_same<T, float>::value, uint32_t, uint64_t>>(
      value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  output.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, typename T>
std::expected<void, std::string> SerializeBinary(std::ostream& output,
                                                 std::span<const T> values) {
  if (std::numeric_limits<SizeType>::max() < values.size()) {
    return std::unexpected(ListSizeError());
  }

  auto list_size_success = SerializeBinary<Endianness, SizeType, SizeType>(
      output, static_cast<SizeType>(values.size()));
  if (!list_size_success) {
    return list_size_success;
  }

  for (const auto& entry : values) {
    auto entry_success = SerializeBinary<Endianness, SizeType>(output, entry);
    if (!entry_success) {
      return entry_success;
    }
  }

  return std::expected<void, std::string>();
}

template <typename T>
std::expected<T, std::string> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<T, std::string> (PlyWriter::*callback)(
        const std::string&, size_t, const std::string&, size_t, uint64_t) const,
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index, uint64_t instance,
    Context& context) {
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance);
}

template <typename T>
std::expected<std::span<const T>, std::string> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<std::span<const T>, std::string> (PlyWriter::*callback)(
        const std::string&, size_t, const std::string&, size_t, uint64_t,
        std::vector<T>&) const,
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index, uint64_t instance,
    Context& context) {
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance,
                                std::get<std::vector<T>>(context));
}

static const std::array<std::string, 16> kTypeNames = {
    "char", "char", "uchar", "uchar", "short", "short", "ushort", "ushort",
    "int",  "int",  "uint",  "uint",  "float", "float", "double", "double"};

void PropertyString(std::ostream& output, size_t type,
                    const std::string& name) {
  output << "property " << kTypeNames.at(type) << " " << name << "\r";
}

void PropertyListString(std::ostream& output, size_t list_type,
                        size_t data_type, const std::string& name) {
  static const std::array<std::string, 16> kListTypeNames = {"uchar", "ushort",
                                                             "uint"};

  output << "property list " << kListTypeNames.at(list_type) << " "
         << kTypeNames.at(data_type) << " " << name << "\r";
}

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::expected<std::function<std::expected<void, std::string>(uint64_t)>,
              std::string>
ToWriteCallback(std::ostream& output, const PlyWriter& ply_writer, T callback,
                const std::string& element_name, size_t element_index,
                const std::string& property_name, size_t property_index,
                Context& context, size_t value_type_index,
                std::optional<size_t> list_size_type) {
  auto property_name_valid = ValidateName(property_name);
  if (!property_name_valid) {
    return std::unexpected(property_name_valid.error());
  }

  if (list_size_type.has_value()) {
    PropertyListString(output, *list_size_type, value_type_index,
                       property_name);
  } else {
    PropertyString(output, value_type_index, property_name);
  }

  if (!output) {
    return std::unexpected(WriteFailure());
  }

  auto result = [&output, &ply_writer, callback, element_name, element_index,
                 property_name, property_index, &context](
                    int64_t instance) -> std::expected<void, std::string> {
    auto result =
        CallCallback(ply_writer, callback, element_name, element_index,
                     property_name, property_index, instance, context);
    if (!result) {
      return std::unexpected(result.error());
    }

    std::expected<void, std::string> write_status;
    if constexpr (Ascii) {
      write_status = SerializeASCII<SizeType>(output, context, *result);
    } else {
      write_status = SerializeBinary<Endianness, SizeType>(output, *result);
    }

    if (!write_status) {
      return write_status;
    }

    if (!output) {
      return std::unexpected(WriteFailure());
    }

    return std::expected<void, std::string>();
  };

  return result;
}

template <bool Ascii, std::endian Endianness, typename T>
std::expected<std::function<std::expected<void, std::string>(uint64_t)>,
              std::string>
ToWriteCallback(std::ostream& output, const PlyWriter& ply_writer, T callback,
                const std::string& element_name, size_t element_index,
                const std::string& property_name, size_t property_index,
                Context& context, size_t value_type_index,
                std::function<std::expected<size_t, std::string>(
                    const std::string&, size_t, const std::string&, size_t)>
                    get_property_list_size_type) {
  using R = std::decay_t<decltype(*CallCallback(
      ply_writer, callback, element_name, element_index, property_name,
      property_index, 0u, context))>;

  std::optional<size_t> maybe_list_size_type;
  if constexpr (std::is_class<R>::value) {
    auto list_size_type = get_property_list_size_type(
        element_name, element_index, property_name, property_index);
    if (!list_size_type) {
      return std::unexpected(list_size_type.error());
    }

    maybe_list_size_type = *list_size_type;

    if (*maybe_list_size_type == 0) {
      return ToWriteCallback<Ascii, Endianness, uint8_t>(
          output, ply_writer, callback, element_name, element_index,
          property_name, property_index, context, value_type_index,
          maybe_list_size_type);
    }

    if (*maybe_list_size_type == 1) {
      return ToWriteCallback<Ascii, Endianness, uint16_t>(
          output, ply_writer, callback, element_name, element_index,
          property_name, property_index, context, value_type_index,
          maybe_list_size_type);
    }
  }

  return ToWriteCallback<Ascii, Endianness, uint32_t>(
      output, ply_writer, callback, element_name, element_index, property_name,
      property_index, context, value_type_index, maybe_list_size_type);
}

template <bool Ascii, std::endian Endianness, typename T>
std::expected<
    std::vector<std::pair<uint64_t, std::vector<std::function<std::expected<
                                        void, std::string>(int64_t)>>>>,
    std::string>
WriteHeader(std::ostream& output, const PlyWriter& ply_writer,
            const std::function<std::expected<size_t, std::string>(
                const std::string&, size_t, const std::string&, size_t)>&
                get_property_list_size_type,
            const char* format,
            std::map<std::string, uint64_t>& num_element_instances,
            const std::map<std::string, std::map<std::string, T>>& callbacks,
            const std::vector<std::string>& comments,
            const std::vector<std::string>& object_info, Context& context) {
  output << "ply\rformat " << format << " 1.0\r";
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  for (const auto& comment : comments) {
    if (!ValidateComment(comment)) {
      return std::unexpected(
          "A comment may not contain line feed or carriage return");
    }

    output << "comment " << comment << "\r";
    if (!output) {
      return std::unexpected(WriteFailure());
    }
  }

  for (const auto& info : object_info) {
    if (!ValidateComment(info)) {
      return std::unexpected(
          "An obj_info may not contain line feed or carriage return");
    }

    output << "obj_info " << info << "\r";
    if (!output) {
      return std::unexpected(WriteFailure());
    }
  }

  std::vector<std::pair<
      uint64_t,
      std::vector<std::function<std::expected<void, std::string>(int64_t)>>>>
      actual_callbacks;
  for (const auto& element : callbacks) {
    auto element_name_valid = ValidateName(element.first);
    if (!element_name_valid) {
      return std::unexpected(element_name_valid.error());
    }

    output << "element " << element.first << " "
           << num_element_instances[element.first] << "\r";
    if (!output) {
      return std::unexpected(WriteFailure());
    }

    std::vector<std::function<std::expected<void, std::string>(int64_t)>> row;
    for (const auto& property : element.second) {
      auto write_callback = std::visit(
          [&](auto callback) {
            return ToWriteCallback<Ascii, Endianness>(
                output, ply_writer, callback, element.first,
                actual_callbacks.size(), property.first, row.size(), context,
                property.second.index(), get_property_list_size_type);
          },
          property.second);
      if (!write_callback) {
        return std::unexpected(write_callback.error());
      }

      row.emplace_back(std::move(*write_callback));
    }
    actual_callbacks.emplace_back(num_element_instances.at(element.first),
                                  std::move(row));
  }

  output << "end_header\r";
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return actual_callbacks;
}

template <std::endian Endianness, typename T>
std::expected<void, std::string> WriteToBinaryImpl(
    std::ostream& output, const PlyWriter& ply_writer,
    const std::function<std::expected<size_t, std::string>(
        const std::string&, size_t, const std::string&, size_t)>&
        get_property_list_size_type,
    std::map<std::string, uint64_t>& num_element_instances,
    const std::map<std::string, std::map<std::string, T>>& callbacks,
    const std::vector<std::string>& comments,
    const std::vector<std::string>& object_info) {
  const char* format;
  if constexpr (Endianness == std::endian::big) {
    format = "binary_big_endian";
  } else {
    format = "binary_little_endian";
  }

  Context context;
  auto actual_callbacks = WriteHeader<false, Endianness>(
      output, ply_writer, get_property_list_size_type, format,
      num_element_instances, callbacks, comments, object_info, context);
  if (!actual_callbacks) {
    return std::unexpected(actual_callbacks.error());
  }

  for (const auto& element : *actual_callbacks) {
    for (uint64_t instance = 0; instance < element.first; instance++) {
      for (const auto& property : element.second) {
        auto result = property(instance);
        if (!result) {
          return std::unexpected(result.error());
        }
      }
    }
  }

  return std::expected<void, std::string>();
}

}  // namespace

std::expected<void, std::string> PlyWriter::WriteTo(
    std::ostream& stream) const {
  if constexpr (std::endian::native == std::endian::big) {
    return WriteToBigEndian(stream);
  } else {
    return WriteToLittleEndian(stream);
  }
}

std::expected<void, std::string> PlyWriter::WriteToASCII(
    std::ostream& stream) const {
  std::map<std::string, uint64_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  auto result = Start(num_element_instances, callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  Context context;
  auto actual_callbacks = WriteHeader<true, std::endian::native>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::string> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      "ascii", num_element_instances, callbacks, comments, object_info,
      context);
  if (!actual_callbacks) {
    return std::unexpected(actual_callbacks.error());
  }

  for (const auto& element : *actual_callbacks) {
    for (uint64_t instance = 0; instance < element.first; instance++) {
      bool first = true;
      for (const auto& property : element.second) {
        if (!first) {
          stream << ' ';
          if (!stream) {
            return std::unexpected(WriteFailure());
          }
        }

        first = false;

        auto result = property(instance);
        if (!result) {
          return std::unexpected(result.error());
        }
      }

      stream << '\r';
      if (!stream) {
        return std::unexpected(WriteFailure());
      }
    }
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> PlyWriter::WriteToBigEndian(
    std::ostream& stream) const {
  std::map<std::string, uint64_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  auto result = Start(num_element_instances, callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  return WriteToBinaryImpl<std::endian::big>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::string> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      num_element_instances, callbacks, comments, object_info);
}

std::expected<void, std::string> PlyWriter::WriteToLittleEndian(
    std::ostream& stream) const {
  std::map<std::string, uint64_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> callbacks;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  auto result = Start(num_element_instances, callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  return WriteToBinaryImpl<std::endian::little>(
      stream, *this,
      [&](const std::string& element_name, size_t element_index,
          const std::string& property_name,
          size_t property_index) -> std::expected<size_t, std::string> {
        auto result = this->GetPropertyListSizeType(
            element_name, element_index, property_name, property_index);
        if (!result) {
          return std::unexpected(result.error());
        }
        return static_cast<size_t>(*result);
      },
      num_element_instances, callbacks, comments, object_info);
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
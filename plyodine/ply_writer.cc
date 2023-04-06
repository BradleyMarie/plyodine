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
#include <type_traits>
#include <vector>

namespace plyodine {
namespace {

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::stringstream>
    Context;

std::string_view FloatingPointError() {
  return "Only finite floating point values may be serialized to an ASCII "
         "output";
}

std::string_view ListSizeError() {
  return "The list was too big to be represented with the selected size type";
}

std::string_view WriteFailure() { return "Write failure"; }

std::expected<void, std::string_view> ValidateName(std::string_view name) {
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

  return std::expected<void, std::string_view>();
}

bool ValidateComment(const std::string_view comment) {
  for (char c : comment) {
    if (c == '\r' || c == '\n') {
      return false;
    }
  }

  return true;
}

std::expected<void, std::string_view> StartHeader(
    std::ostream& output, std::string_view format,
    std::span<const std::string> comments,
    std::span<const std::string> object_info) {
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

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> EndHeader(std::ostream& output) {
  output << "end_header\r";
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string_view>();
}

template <std::integral SizeType, std::integral T>
std::expected<void, std::string_view> SerializeASCII(std::ostream& output,
                                                     Context& context,
                                                     T value) {
  output << +value;
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string_view>();
}

template <std::integral SizeType, std::floating_point T>
std::expected<void, std::string_view> SerializeASCII(std::ostream& output,
                                                     Context& context,
                                                     T value) {
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

  return std::expected<void, std::string_view>();
}

template <std::integral SizeType, typename T>
std::expected<void, std::string_view> SerializeASCII(
    std::ostream& output, Context& context, std::span<const T> values) {
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

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness, std::integral SizeType, std::integral T>
std::expected<void, std::string_view> SerializeBinary(std::ostream& output,
                                                      T value) {
  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  output.write(reinterpret_cast<char*>(&value), sizeof(value));
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness, std::integral SizeType>
std::expected<void, std::string_view> SerializeBinary(std::ostream& output,
                                                      float value) {
  auto entry = std::bit_cast<uint32_t>(value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  output.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness, std::integral SizeType>
std::expected<void, std::string_view> SerializeBinary(std::ostream& output,
                                                      double value) {
  auto entry = std::bit_cast<uint64_t>(value);

  if (Endianness != std::endian::native) {
    entry = std::byteswap(entry);
  }

  output.write(reinterpret_cast<char*>(&entry), sizeof(entry));
  if (!output) {
    return std::unexpected(WriteFailure());
  }

  return std::expected<void, std::string_view>();
}

template <std::endian Endianness, std::integral SizeType, typename T>
std::expected<void, std::string_view> SerializeBinary(
    std::ostream& output, std::span<const T> values) {
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

  return std::expected<void, std::string_view>();
}

template <typename T>
std::expected<T, std::string_view> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<T, std::string_view> (PlyWriter::*callback)(
        std::string_view, size_t, std::string_view, size_t, uint64_t) const,
    std::string_view element_name, size_t element_index,
    std::string_view property_name, size_t property_index, uint64_t instance,
    Context& context) {
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance);
}

template <typename T>
std::expected<std::span<const T>, std::string_view> CallCallback(
    const PlyWriter& ply_writer,
    std::expected<std::span<const T>, std::string_view> (PlyWriter::*callback)(
        std::string_view, size_t, std::string_view, size_t, uint64_t,
        std::vector<T>&) const,
    std::string_view element_name, size_t element_index,
    std::string_view property_name, size_t property_index, uint64_t instance,
    Context& context) {
  return (ply_writer.*callback)(element_name, element_index, property_name,
                                property_index, instance,
                                std::get<std::vector<T>>(context));
}

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::expected<std::function<std::expected<void, std::string_view>(uint64_t)>,
              std::string_view>
ToWriteCallback(std::ostream& output, const PlyWriter& ply_writer, T callback,
                std::string_view element_name, size_t element_index,
                std::string_view property_name, size_t property_index,
                Context& context, PropertyType value_type_index,
                std::optional<PropertyType> list_size_type) {
  static const std::array<std::string_view, 16> type_names = {
      "char", "char", "uchar", "uchar", "short", "short", "ushort", "ushort",
      "int",  "int",  "uint",  "uint",  "float", "float", "double", "double"};

  auto property_name_valid = ValidateName(property_name);
  if (!property_name_valid) {
    return std::unexpected(property_name_valid.error());
  }

  if (list_size_type.has_value()) {
    output << "property list "
           << type_names.at(static_cast<size_t>(*list_size_type)) << " "
           << type_names.at(static_cast<size_t>(value_type_index)) << " "
           << property_name << "\r";
  } else {
    output << "property "
           << type_names.at(static_cast<size_t>(value_type_index)) << " "
           << property_name << "\r";
  }

  if (!output) {
    return std::unexpected(WriteFailure());
  }

  auto result = [&output, &ply_writer, callback, element_name, element_index,
                 property_name, property_index, &context](
                    int64_t instance) -> std::expected<void, std::string_view> {
    auto result =
        CallCallback(ply_writer, callback, element_name, element_index,
                     property_name, property_index, instance, context);
    if (!result) {
      return std::unexpected(result.error());
    }

    std::expected<void, std::string_view> write_status;
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

    return std::expected<void, std::string_view>();
  };

  return result;
}

template <bool Ascii, std::endian Endianness, typename T>
std::expected<std::function<std::expected<void, std::string_view>(uint64_t)>,
              std::string_view>
ToWriteCallback(std::ostream& output, const PlyWriter& ply_writer, T callback,
                std::string_view element_name, size_t element_index,
                std::string_view property_name, size_t property_index,
                Context& context, PropertyType value_type_index,
                std::function<std::expected<PropertyType, std::string_view>(
                    std::string_view, size_t, std::string_view, size_t)>
                    get_property_list_size_type) {
  using R = std::decay_t<decltype(*CallCallback(
      ply_writer, callback, element_name, element_index, property_name,
      property_index, 0u, context))>;

  std::optional<PropertyType> maybe_list_size_type;
  if constexpr (std::is_class<R>::value) {
    auto list_size_type = get_property_list_size_type(
        element_name, element_index, property_name, property_index);
    if (!list_size_type) {
      return std::unexpected(list_size_type.error());
    }

    maybe_list_size_type = *list_size_type;

    if (*maybe_list_size_type == PropertyType::UINT8) {
      return ToWriteCallback<Ascii, Endianness, uint8_t>(
          output, ply_writer, callback, element_name, element_index,
          property_name, property_index, context, value_type_index,
          maybe_list_size_type);
    }

    if (*maybe_list_size_type == PropertyType::UINT16) {
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

}  // namespace

std::expected<void, std::string_view> PlyWriter::WriteTo(
    std::ostream& stream) const {
  // Static assertions to ensure variants of Callback are properly ordered
  static_assert(Callback(Int8PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT8));
  static_assert(Callback(Int8PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT8_LIST));
  static_assert(Callback(UInt8PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT8));
  static_assert(Callback(UInt8PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT8_LIST));
  static_assert(Callback(Int16PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT16));
  static_assert(Callback(Int16PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT16_LIST));
  static_assert(Callback(UInt16PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT16));
  static_assert(Callback(UInt16PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT16_LIST));
  static_assert(Callback(Int32PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT32));
  static_assert(Callback(Int32PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::INT32_LIST));
  static_assert(Callback(UInt32PropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT32));
  static_assert(Callback(UInt32PropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::UINT32_LIST));
  static_assert(Callback(FloatPropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::FLOAT));
  static_assert(Callback(FloatPropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::FLOAT_LIST));
  static_assert(Callback(DoublePropertyCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::DOUBLE));
  static_assert(Callback(DoublePropertyListCallback(nullptr)).index() ==
                static_cast<size_t>(PropertyType::DOUBLE_LIST));

  if constexpr (std::endian::native == std::endian::big) {
    return WriteToBigEndian(stream);
  } else {
    return WriteToLittleEndian(stream);
  }
}

std::expected<void, std::string_view> PlyWriter::WriteToASCII(
    std::ostream& stream) const {
  std::map<std::string_view,
           std::pair<uint64_t, std::map<std::string_view, Callback>>>
      property_callbacks;
  std::span<const std::string> comments;
  std::span<const std::string> object_info;
  auto result = Start(property_callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  auto header_started = StartHeader(stream, "ascii", comments, object_info);
  if (!header_started) {
    return header_started;
  }

  Context context;
  std::vector<std::pair<uint64_t, std::vector<std::function<std::expected<
                                      void, std::string_view>(int64_t)>>>>
      callbacks;
  for (const auto& element : property_callbacks) {
    auto element_name_valid = ValidateName(element.first);
    if (!element_name_valid) {
      return std::unexpected(element_name_valid.error());
    }

    stream << "element " << element.first << " " << element.second.first
           << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }

    std::vector<std::function<std::expected<void, std::string_view>(int64_t)>>
        row;
    for (const auto& property : element.second.second) {
      auto write_callback = std::visit(
          [&](auto callback) {
            return ToWriteCallback<true, std::endian::native>(
                stream, *this, callback, element.first, callbacks.size(),
                property.first, row.size(), context,
                static_cast<PropertyType>(property.second.index()),
                [&](std::string_view element_name, size_t element_index,
                    std::string_view property_name, size_t property_index)
                    -> std::expected<PropertyType, std::string_view> {
                  auto result = this->GetPropertyListSizeType(
                      element_name, element_index, property_name,
                      property_index);
                  if (!result) {
                    return std::unexpected(result.error());
                  }
                  return static_cast<PropertyType>(*result);
                });
          },
          property.second);
      if (!write_callback) {
        return std::unexpected(write_callback.error());
      }

      row.emplace_back(std::move(*write_callback));
    }
    callbacks.emplace_back(element.second.first, std::move(row));
  }

  auto header_ended = EndHeader(stream);
  if (!header_ended) {
    return header_ended;
  }

  for (const auto& element : callbacks) {
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

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> PlyWriter::WriteToBigEndian(
    std::ostream& stream) const {
  std::map<std::string_view,
           std::pair<uint64_t, std::map<std::string_view, Callback>>>
      property_callbacks;
  std::span<const std::string> comments;
  std::span<const std::string> object_info;
  auto result = Start(property_callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  auto header_started =
      StartHeader(stream, "binary_big_endian", comments, object_info);
  if (!header_started) {
    return header_started;
  }

  Context context;
  std::vector<std::pair<uint64_t, std::vector<std::function<std::expected<
                                      void, std::string_view>(int64_t)>>>>
      callbacks;
  for (const auto& element : property_callbacks) {
    auto element_name_valid = ValidateName(element.first);
    if (!element_name_valid) {
      return std::unexpected(element_name_valid.error());
    }

    stream << "element " << element.first << " " << element.second.first
           << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }

    std::vector<std::function<std::expected<void, std::string_view>(int64_t)>>
        row;
    for (const auto& property : element.second.second) {
      auto write_callback = std::visit(
          [&](auto callback) {
            return ToWriteCallback<false, std::endian::big>(
                stream, *this, callback, element.first, callbacks.size(),
                property.first, row.size(), context,
                static_cast<PropertyType>(property.second.index()),
                [&](std::string_view element_name, size_t element_index,
                    std::string_view property_name, size_t property_index)
                    -> std::expected<PropertyType, std::string_view> {
                  auto result = this->GetPropertyListSizeType(
                      element_name, element_index, property_name,
                      property_index);
                  if (!result) {
                    return std::unexpected(result.error());
                  }
                  return static_cast<PropertyType>(*result);
                });
          },
          property.second);
      if (!write_callback) {
        return std::unexpected(write_callback.error());
      }

      row.emplace_back(std::move(*write_callback));
    }
    callbacks.emplace_back(element.second.first, std::move(row));
  }

  auto header_ended = EndHeader(stream);
  if (!header_ended) {
    return header_ended;
  }

  for (const auto& element : callbacks) {
    for (uint64_t instance = 0; instance < element.first; instance++) {
      for (const auto& property : element.second) {
        auto result = property(instance);
        if (!result) {
          return std::unexpected(result.error());
        }
      }
    }
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> PlyWriter::WriteToLittleEndian(
    std::ostream& stream) const {
  std::map<std::string_view,
           std::pair<uint64_t, std::map<std::string_view, Callback>>>
      property_callbacks;
  std::span<const std::string> comments;
  std::span<const std::string> object_info;
  auto result = Start(property_callbacks, comments, object_info);
  if (!result) {
    return result;
  }

  auto header_started =
      StartHeader(stream, "binary_little_endian", comments, object_info);
  if (!header_started) {
    return header_started;
  }

  Context context;
  std::vector<std::pair<uint64_t, std::vector<std::function<std::expected<
                                      void, std::string_view>(int64_t)>>>>
      callbacks;
  for (const auto& element : property_callbacks) {
    auto element_name_valid = ValidateName(element.first);
    if (!element_name_valid) {
      return std::unexpected(element_name_valid.error());
    }

    stream << "element " << element.first << " " << element.second.first
           << "\r";
    if (!stream) {
      return std::unexpected(WriteFailure());
    }

    std::vector<std::function<std::expected<void, std::string_view>(int64_t)>>
        row;
    for (const auto& property : element.second.second) {
      auto write_callback = std::visit(
          [&](auto callback) {
            return ToWriteCallback<false, std::endian::little>(
                stream, *this, callback, element.first, callbacks.size(),
                property.first, row.size(), context,
                static_cast<PropertyType>(property.second.index()),
                [&](std::string_view element_name, size_t element_index,
                    std::string_view property_name, size_t property_index)
                    -> std::expected<PropertyType, std::string_view> {
                  auto result = this->GetPropertyListSizeType(
                      element_name, element_index, property_name,
                      property_index);
                  if (!result) {
                    return std::unexpected(result.error());
                  }
                  return static_cast<PropertyType>(*result);
                });
          },
          property.second);
      if (!write_callback) {
        return std::unexpected(write_callback.error());
      }

      row.emplace_back(std::move(*write_callback));
    }
    callbacks.emplace_back(element.second.first, std::move(row));
  }

  auto header_ended = EndHeader(stream);
  if (!header_ended) {
    return header_ended;
  }

  for (const auto& element : callbacks) {
    for (uint64_t instance = 0; instance < element.first; instance++) {
      for (const auto& property : element.second) {
        auto result = property(instance);
        if (!result) {
          return std::unexpected(result.error());
        }
      }
    }
  }

  return std::expected<void, std::string_view>();
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <functional>
#include <limits>
#include <sstream>
#include <tuple>
#include <type_traits>

#include "plyodine/ply_header_reader.h"

namespace plyodine {
namespace {

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::string_view,
                   std::stringstream, std::string, bool>
    Context;

std::string UnexpectedEOF() { return "Unexpected EOF"; }

std::string NegativeListSize() {
  return "The input contained a property list with a negative size";
}

std::expected<void, std::string> ReadNextLine(std::istream& input,
                                              Context& context) {
  auto line_ending = std::get<std::string_view>(context);
  auto& line = std::get<std::stringstream>(context);

  line.str("");
  line.clear();

  char c;
  while (input.get(c)) {
    if (c == line_ending[0]) {
      line_ending.remove_prefix(1);

      while (!line_ending.empty() && input.get(c)) {
        if (c != line_ending[0]) {
          return std::unexpected("The input contained mismatched line endings");
        }
        line_ending.remove_prefix(1);
      }

      break;
    }

    if (c != ' ' && !std::isgraph(c)) {
      return std::unexpected("The input contained an invalid character");
    }

    line.put(c);
  }

  std::get<bool>(context) = input.eof();

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ReadNextToken(Context& context) {
  auto& input = std::get<std::stringstream>(context);
  auto& token = std::get<std::string>(context);
  bool last_line = std::get<bool>(context);

  token.clear();

  char c = 0;
  while (input.get(c)) {
    if (c == ' ') {
      break;
    }

    token.push_back(c);
  }

  if (!c) {
    if (last_line) {
      return std::unexpected(UnexpectedEOF());
    }

    return std::unexpected(
        "The input contained an element with too few tokens");
  }

  if (token.empty()) {
    return std::unexpected("The input contained an empty token");
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> CheckForUnusedTokens(Context& context) {
  auto& line = std::get<std::stringstream>(context);

  char c;
  while (line.get(c)) {
    if (std::isgraph(c)) {
      return std::unexpected(
          "The input contained an element with unused tokens");
    }
  }

  return std::expected<void, std::string>();
}

template <std::integral SizeType, bool IsListSize, typename T>
std::expected<void, std::string> DeserializeASCII(std::istream& input,
                                                  Context& context, T* value) {
  auto success = ReadNextToken(context);
  if (!success) {
    return std::unexpected(success.error());
  }

  auto& token = std::get<std::string>(context);

  auto parsing_result =
      std::from_chars(token.data(), token.data() + token.size(), *value);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    if constexpr (IsListSize) {
      return std::unexpected(
          "The input contained a property list size that was out of range");
    }
    return std::unexpected(
        "The input contained a property entry that was out of range");
  } else if (parsing_result.ec != std::errc{}) {
    if constexpr (IsListSize) {
      return std::unexpected(
          "The input contained an unparsable property list size");
    }
    return std::unexpected("The input contained an unparsable property entry");
  }

  return std::expected<void, std::string>();
}

template <std::integral SizeType, bool IsListSize, typename T>
std::expected<void, std::string> DeserializeASCII(std::istream& input,
                                                  Context& context,
                                                  std::span<const T>* value) {
  SizeType list_size;
  auto list_size_error =
      DeserializeASCII<SizeType, true>(input, context, &list_size);
  if (!list_size_error) {
    return list_size_error;
  }

  if constexpr (std::is_signed<SizeType>::value) {
    if (list_size < 0) {
      return std::unexpected(NegativeListSize());
    }
  }

  auto& result = std::get<std::vector<T>>(context);
  result.clear();

  for (SizeType i = 0; i < list_size; i++) {
    T entry;
    auto value_error =
        DeserializeASCII<SizeType, false>(input, context, &entry);
    if (!value_error) {
      return value_error;
    }

    result.push_back(entry);
  }

  *value = std::span<const T>(result);

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, std::integral T>
std::expected<void, std::string> DeserializeBinary(std::istream& input,
                                                   Context& context, T* value) {
  input.read(reinterpret_cast<char*>(value), sizeof(*value));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if (Endianness != std::endian::native) {
    *value = std::byteswap(*value);
  }

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, std::floating_point T>
std::expected<void, std::string> DeserializeBinary(std::istream& input,
                                                   Context& context, T* value) {
  std::conditional_t<std::is_same<T, float>::value, uint32_t, uint64_t> read;

  input.read(reinterpret_cast<char*>(&read), sizeof(read));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if (Endianness != std::endian::native) {
    read = std::byteswap(read);
  }

  *value = std::bit_cast<T>(read);

  return std::expected<void, std::string>();
}

template <std::endian Endianness, std::integral SizeType, typename T>
std::expected<void, std::string> DeserializeBinary(std::istream& input,
                                                   Context& context,
                                                   std::span<const T>* value) {
  SizeType list_size;
  auto list_size_error =
      DeserializeBinary<Endianness, SizeType>(input, context, &list_size);
  if (!list_size_error) {
    return list_size_error;
  }

  if constexpr (std::is_signed<SizeType>::value) {
    if (list_size < 0) {
      return std::unexpected(NegativeListSize());
    }
  }

  auto& result = std::get<std::vector<T>>(context);
  result.clear();

  for (SizeType i = 0; i < list_size; i++) {
    T entry;
    auto value_error =
        DeserializeBinary<Endianness, SizeType>(input, context, &entry);
    if (!value_error) {
      return value_error;
    }

    result.push_back(entry);
  }

  *value = std::span<const T>(result);

  return std::expected<void, std::string>();
}

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::function<std::expected<void, std::string>(uint64_t)> ToReadCallback(
    std::istream& input, PlyReader& ply_reader,
    std::expected<void, std::string> (PlyReader::*callback)(
        const std::string&, size_t, const std::string&, size_t, uint64_t, T),
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index,
    size_t actual_property_index, size_t num_properties, Context& context) {
  auto result =
      [&input, &ply_reader, callback, element_name, element_index,
       property_name, property_index, actual_property_index, num_properties,
       &context](uint64_t instance) -> std::expected<void, std::string> {
    if constexpr (Ascii) {
      if (actual_property_index == 0) {
        auto read_line = ReadNextLine(input, context);
        if (!read_line) {
          return read_line;
        }
      }
    }

    T result;

    std::expected<void, std::string> error;
    if constexpr (Ascii) {
      error = DeserializeASCII<SizeType, false>(input, context, &result);
    } else {
      error = DeserializeBinary<Endianness, SizeType>(input, context, &result);
    }

    if (!error) {
      return error;
    }

    if (callback == nullptr) {
      return std::expected<void, std::string>();
    }

    auto callback_success =
        (ply_reader.*callback)(element_name, element_index, property_name,
                               property_index, instance, result);
    if (!callback_success) {
      return callback_success;
    }

    if constexpr (Ascii) {
      if (actual_property_index + 1 == num_properties) {
        auto unused_tokens_error = CheckForUnusedTokens(context);
        if (!unused_tokens_error) {
          return unused_tokens_error;
        }
      }
    }

    return std::expected<void, std::string>();
  };

  return result;
}

template <bool Ascii, std::endian Endianness, typename T>
std::function<std::expected<void, std::string>(uint64_t)> ToReadCallback(
    std::istream& input, PlyReader& ply_reader,
    std::expected<void, std::string> (PlyReader::*callback)(
        const std::string&, size_t, const std::string&, size_t, uint64_t, T),
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index,
    size_t actual_property_index, size_t num_properties,
    std::optional<PlyHeader::Property::Type> list_type, Context& context) {
  if constexpr (std::is_class<T>::value) {
    if (list_type.has_value()) {
      switch (*list_type) {
        case PlyHeader::Property::INT8:
          return ToReadCallback<Ascii, Endianness, int8_t>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT8:
          return ToReadCallback<Ascii, Endianness, uint8_t>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::INT16:
          return ToReadCallback<Ascii, Endianness, int16_t>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT16:
          return ToReadCallback<Ascii, Endianness, uint16_t>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::INT32:
          return ToReadCallback<Ascii, Endianness, int32_t>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT32:
          break;
        default:
          assert(false);
      }
    }
  }

  return ToReadCallback<Ascii, Endianness, uint32_t>(
      input, ply_reader, callback, element_name, element_index, property_name,
      property_index, actual_property_index, num_properties, context);
}

PropertyType ToPropertyType(const PlyHeader::Property& property) {
  int list_offset = property.list_type.has_value() ? 1 : 0;
  return static_cast<PropertyType>(2 * property.data_type + list_offset);
}

template <bool Ascii, std::endian Endianness, typename T>
std::vector<std::pair<
    uint64_t,
    std::vector<std::function<std::expected<void, std::string>(uint64_t)>>>>
BuildCallbacks(
    std::istream& input, PlyReader& ply_reader, const PlyHeader& ply_header,
    const std::map<std::string, std::map<std::string, T>>& property_callbacks,
    Context& context) {
  std::vector<std::pair<
      uint64_t,
      std::vector<std::function<std::expected<void, std::string>(uint64_t)>>>>
      result;

  size_t element_index = 0;
  for (const auto& element : ply_header.elements) {
    std::vector<std::function<std::expected<void, std::string>(uint64_t)>>
        callbacks;
    size_t property_index = 0;
    bool had_properties = false;
    for (size_t actual_property_index = 0;
         actual_property_index < element.properties.size();
         actual_property_index++) {
      const auto& property = element.properties.at(actual_property_index);
      PropertyType type = ToPropertyType(property);

      T callback;
      switch (type) {
        case PropertyType::INT8:
          callback.template emplace<static_cast<size_t>(PropertyType::INT8)>(
              nullptr);
          break;
        case PropertyType::INT8_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::INT8_LIST)>(
                  nullptr);
          break;
        case PropertyType::UINT8:
          callback.template emplace<static_cast<size_t>(PropertyType::UINT8)>(
              nullptr);
          break;
        case PropertyType::UINT8_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::UINT8_LIST)>(
                  nullptr);
          break;
        case PropertyType::INT16:
          callback.template emplace<static_cast<size_t>(PropertyType::INT16)>(
              nullptr);
          break;
        case PropertyType::INT16_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::INT16_LIST)>(
                  nullptr);
          break;
        case PropertyType::UINT16:
          callback.template emplace<static_cast<size_t>(PropertyType::UINT16)>(
              nullptr);
          break;
        case PropertyType::UINT16_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::UINT16_LIST)>(
                  nullptr);
          break;
        case PropertyType::INT32:
          callback.template emplace<static_cast<size_t>(PropertyType::INT32)>(
              nullptr);
          break;
        case PropertyType::INT32_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::INT32_LIST)>(
                  nullptr);
          break;
        case PropertyType::UINT32:
          callback.template emplace<static_cast<size_t>(PropertyType::UINT32)>(
              nullptr);
          break;
        case PropertyType::UINT32_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::UINT32_LIST)>(
                  nullptr);
          break;
        case PropertyType::FLOAT:
          callback.template emplace<static_cast<size_t>(PropertyType::FLOAT)>(
              nullptr);
          break;
        case PropertyType::FLOAT_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::FLOAT_LIST)>(
                  nullptr);
          break;
        case PropertyType::DOUBLE:
          callback.template emplace<static_cast<size_t>(PropertyType::DOUBLE)>(
              nullptr);
          break;
        case PropertyType::DOUBLE_LIST:
          callback
              .template emplace<static_cast<size_t>(PropertyType::DOUBLE_LIST)>(
                  nullptr);
          break;
      }

      size_t current_property_index = property_index;

      auto element_iter = property_callbacks.find(element.name);
      if (element_iter != property_callbacks.end()) {
        auto property_iter = element_iter->second.find(property.name);
        if (property_iter != element_iter->second.end()) {
          if (property_iter->second.index() == static_cast<size_t>(type)) {
            callback = property_iter->second;
            property_index += 1u;
            had_properties = true;
          }
        }
      }

      callbacks.emplace_back(std::visit(
          [&](const auto& func_ptr) {
            return ToReadCallback<Ascii, Endianness>(
                input, ply_reader, func_ptr, element.name, element_index,
                property.name, current_property_index, actual_property_index,
                element.properties.size(), property.list_type, context);
          },
          callback));
    }

    if (had_properties) {
      element_index += 1;
    }

    result.emplace_back(element.num_in_file, std::move(callbacks));
  }

  return result;
}

}  // namespace

std::expected<void, std::string> PlyReader::ReadFrom(std::istream& input) {
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

  auto header = ReadPlyHeader(input);
  if (!header) {
    return std::unexpected(header.error());
  }

  std::map<std::string,
           std::pair<uint64_t, std::map<std::string, PropertyType>>>
      all_properties;
  for (const auto& element : header->elements) {
    std::map<std::string, PropertyType> properties;
    for (const auto& property : element.properties) {
      if (property.list_type) {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] = PropertyType::INT8_LIST;
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] = PropertyType::UINT8_LIST;
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] = PropertyType::INT16_LIST;
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] = PropertyType::UINT16_LIST;
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] = PropertyType::INT32_LIST;
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] = PropertyType::UINT32_LIST;
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] = PropertyType::FLOAT_LIST;
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] = PropertyType::DOUBLE_LIST;
            break;
        }
      } else {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            properties[property.name] = PropertyType::INT8;
            break;
          case PlyHeader::Property::UINT8:
            properties[property.name] = PropertyType::UINT8;
            break;
          case PlyHeader::Property::INT16:
            properties[property.name] = PropertyType::INT16;
            break;
          case PlyHeader::Property::UINT16:
            properties[property.name] = PropertyType::UINT16;
            break;
          case PlyHeader::Property::INT32:
            properties[property.name] = PropertyType::INT32;
            break;
          case PlyHeader::Property::UINT32:
            properties[property.name] = PropertyType::UINT32;
            break;
          case PlyHeader::Property::FLOAT:
            properties[property.name] = PropertyType::FLOAT;
            break;
          case PlyHeader::Property::DOUBLE:
            properties[property.name] = PropertyType::DOUBLE;
            break;
        }
      }
    }
    all_properties[element.name] =
        std::make_pair(element.num_in_file, std::move(properties));
  }

  auto started = Start(all_properties, header->comments, header->object_info);
  if (!started) {
    return std::unexpected(started.error());
  }

  Context context;
  std::get<std::string_view>(context) = header->line_ending;

  std::vector<std::pair<
      uint64_t,
      std::vector<std::function<std::expected<void, std::string>(uint64_t)>>>>
      callbacks;
  switch (header->format) {
    case PlyHeader::ASCII:
      callbacks = BuildCallbacks<true, std::endian::native>(
          input, *this, *header, *started, context);
      break;
    case PlyHeader::BINARY_BIG_ENDIAN:
      callbacks = BuildCallbacks<false, std::endian::big>(input, *this, *header,
                                                          *started, context);
      break;
    case PlyHeader::BINARY_LITTLE_ENDIAN:
      callbacks = BuildCallbacks<false, std::endian::little>(
          input, *this, *header, *started, context);
      break;
  }

  for (const auto& element : callbacks) {
    for (uint64_t instance = 0; instance < element.first; instance++) {
      for (const auto& property : element.second) {
        auto result = property(instance);
        if (!result) {
          return result;
        }
      }
    }
  }

  return std::expected<void, std::string>();
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
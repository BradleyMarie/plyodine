#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <limits>
#include <memory>
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

std::string NegativeListSize() {
  return "The input contained a property list with a negative size";
}

std::string UnexpectedEOF() { return "Unexpected EOF"; }

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
  std::conditional_t<std::is_same<T, float>::value, uint32_t, uintmax_t> read;

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

class PropertyReaderBase {
 public:
  virtual std::expected<void, std::string> Read(uintmax_t instance) = 0;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
class PropertyReader final : public PropertyReaderBase {
 public:
  PropertyReader(std::istream& input, PlyReader& ply_reader,
                 std::expected<void, std::string> (PlyReader::*callback)(
                     const std::string&, size_t, const std::string&, size_t,
                     uintmax_t, T),
                 const std::string& element_name, size_t element_index,
                 const std::string& property_name, size_t property_index,
                 size_t actual_property_index, size_t num_properties,
                 Context& context)
      : input_(input),
        ply_reader_(ply_reader),
        callback_(callback),
        element_name_(element_name),
        element_index_(element_index),
        property_name_(property_name),
        property_index_(property_index),
        actual_property_index_(actual_property_index),
        num_properties_(num_properties),
        context_(context) {}

  std::expected<void, std::string> Read(uintmax_t instance) override;

 private:
  std::istream& input_;
  PlyReader& ply_reader_;
  std::expected<void, std::string> (PlyReader::*callback_)(
      const std::string&, size_t, const std::string&, size_t, uintmax_t, T);
  const std::string& element_name_;
  size_t element_index_;
  const std::string& property_name_;
  size_t property_index_;
  size_t actual_property_index_;
  size_t num_properties_;
  Context& context_;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::expected<void, std::string>
PropertyReader<Ascii, Endianness, SizeType, T>::Read(uintmax_t instance) {
  if constexpr (Ascii) {
    if (actual_property_index_ == 0) {
      auto read_line = ReadNextLine(input_, context_);
      if (!read_line) {
        return read_line;
      }
    }
  }

  T result;

  std::expected<void, std::string> error;
  if constexpr (Ascii) {
    error = DeserializeASCII<SizeType, false>(input_, context_, &result);
  } else {
    error = DeserializeBinary<Endianness, SizeType>(input_, context_, &result);
  }

  if (!error) {
    return error;
  }

  if (callback_ == nullptr) {
    return std::expected<void, std::string>();
  }

  auto callback_success =
      (ply_reader_.*callback_)(element_name_, element_index_, property_name_,
                               property_index_, instance, result);
  if (!callback_success) {
    return callback_success;
  }

  if constexpr (Ascii) {
    if (actual_property_index_ + 1 == num_properties_) {
      auto unused_tokens_error = CheckForUnusedTokens(context_);
      if (!unused_tokens_error) {
        return unused_tokens_error;
      }
    }
  }

  return std::expected<void, std::string>();
}

template <bool Ascii, std::endian Endianness, typename T>
std::unique_ptr<PropertyReaderBase> BuildPropertyReader(
    std::istream& input, PlyReader& ply_reader,
    std::expected<void, std::string> (PlyReader::*callback)(
        const std::string&, size_t, const std::string&, size_t, uintmax_t, T),
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index,
    size_t actual_property_index, size_t num_properties,
    std::optional<PlyHeader::Property::Type> list_type, Context& context) {
  if constexpr (std::is_class<T>::value) {
    if (list_type.has_value()) {
      switch (*list_type) {
        case PlyHeader::Property::INT8:
          return std::make_unique<PropertyReader<Ascii, Endianness, int8_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT8:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, uint8_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::INT16:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, int16_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT16:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, uint16_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::INT32:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, int32_t, T>>(
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

  return std::make_unique<PropertyReader<Ascii, Endianness, uint32_t, T>>(
      input, ply_reader, callback, element_name, element_index, property_name,
      property_index, actual_property_index, num_properties, context);
}

template <bool Ascii, std::endian Endianness, typename T>
std::vector<
    std::pair<uintmax_t, std::vector<std::unique_ptr<PropertyReaderBase>>>>
BuildPropertyReaders(
    std::istream& input, PlyReader& ply_reader,
    const std::vector<
        std::tuple<std::string, uintmax_t,
                   std::vector<std::tuple<
                       std::string, std::optional<PlyHeader::Property::Type>,
                       size_t, size_t, T>>>>& ordered_callbacks,
    Context& context) {
  std::vector<
      std::pair<uintmax_t, std::vector<std::unique_ptr<PropertyReaderBase>>>>
      result;
  for (size_t actual_element_index = 0;
       actual_element_index < ordered_callbacks.size();
       actual_element_index++) {
    const auto& element = ordered_callbacks.at(actual_element_index);
    const auto& properties = std::get<2>(element);

    result.emplace_back(std::get<uintmax_t>(element),
                        std::vector<std::unique_ptr<PropertyReaderBase>>());
    for (size_t actual_property_index = 0;
         actual_property_index < properties.size(); actual_property_index++) {
      const auto& property = properties.at(actual_property_index);

      result.back().second.emplace_back(std::visit(
          [&](const auto& func_ptr) {
            return BuildPropertyReader<Ascii, Endianness>(
                input, ply_reader, func_ptr, std::get<0>(element),
                std::get<2>(property), std::get<0>(property),
                std::get<3>(property), actual_property_index, properties.size(),
                std::get<1>(property), context);
          },
          std::get<4>(property)));
    }
  }

  return result;
}

}  // namespace

std::expected<void, std::string> PlyReader::ReadFrom(std::istream& input) {
  auto header = ReadPlyHeader(input);
  if (!header) {
    return std::unexpected(header.error());
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, Callback>> empty_callbacks;
  for (const auto& element : header->elements) {
    num_element_instances[element.name] = element.num_in_file;

    auto insertion_iterator =
        empty_callbacks.emplace(element.name, std::map<std::string, Callback>())
            .first;
    for (const auto& property : element.properties) {
      Callback callback;
      if (property.list_type) {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            callback = Int8PropertyListCallback();
            break;
          case PlyHeader::Property::UINT8:
            callback = UInt8PropertyListCallback();
            break;
          case PlyHeader::Property::INT16:
            callback = Int16PropertyListCallback();
            break;
          case PlyHeader::Property::UINT16:
            callback = UInt16PropertyListCallback();
            break;
          case PlyHeader::Property::INT32:
            callback = Int32PropertyListCallback();
            break;
          case PlyHeader::Property::UINT32:
            callback = UInt32PropertyListCallback();
            break;
          case PlyHeader::Property::FLOAT:
            callback = FloatPropertyListCallback();
            break;
          case PlyHeader::Property::DOUBLE:
            callback = DoublePropertyListCallback();
            break;
        }
      } else {
        switch (property.data_type) {
          case PlyHeader::Property::INT8:
            callback = Int8PropertyCallback();
            break;
          case PlyHeader::Property::UINT8:
            callback = UInt8PropertyCallback();
            break;
          case PlyHeader::Property::INT16:
            callback = Int16PropertyCallback();
            break;
          case PlyHeader::Property::UINT16:
            callback = UInt16PropertyCallback();
            break;
          case PlyHeader::Property::INT32:
            callback = Int32PropertyCallback();
            break;
          case PlyHeader::Property::UINT32:
            callback = UInt32PropertyCallback();
            break;
          case PlyHeader::Property::FLOAT:
            callback = FloatPropertyCallback();
            break;
          case PlyHeader::Property::DOUBLE:
            callback = DoublePropertyCallback();
            break;
        }
      }

      insertion_iterator->second[property.name] = callback;
    }
  }

  auto callbacks = empty_callbacks;
  auto started = Start(num_element_instances, callbacks, header->comments,
                       header->object_info);
  if (!started) {
    return std::unexpected(started.error());
  }

  size_t element_index = 0;
  std::map<std::string,
           std::map<std::string, std::tuple<size_t, size_t, Callback>>>
      requested_callbacks;
  for (const auto& element : callbacks) {
    size_t property_index = 0;
    for (const auto& property : element.second) {
      if (std::visit([](auto value) { return value == nullptr; },
                     property.second)) {
        continue;
      }

      requested_callbacks[element.first][property.first] =
          std::make_tuple(element_index, property_index, property.second);
      property_index += 1;
    }

    if (property_index != 0) {
      element_index += 1;
    }
  }

  std::map<std::string,
           std::map<std::string, std::tuple<size_t, size_t, Callback>>>
      numbered_callbacks;
  for (const auto& element : empty_callbacks) {
    for (const auto& property : element.second) {
      auto insertion_iterator =
          numbered_callbacks[element.first]
              .emplace(property.first, std::make_tuple(0, 0, property.second))
              .first;

      auto element_iter = requested_callbacks.find(element.first);
      if (element_iter == requested_callbacks.end()) {
        continue;
      }

      auto property_iter = element_iter->second.find(property.first);
      if (property_iter == element_iter->second.end()) {
        continue;
      }

      if (std::get<2>(property_iter->second).index() !=
          property.second.index()) {
        continue;
      }

      insertion_iterator->second = property_iter->second;
    }
  }

  std::vector<
      std::tuple<std::string, uintmax_t,
                 std::vector<std::tuple<
                     std::string, std::optional<PlyHeader::Property::Type>,
                     size_t, size_t, Callback>>>>
      ordered_callbacks;
  for (const auto& element : header->elements) {
    ordered_callbacks.emplace_back(
        element.name, element.num_in_file,
        std::vector<
            std::tuple<std::string, std::optional<PlyHeader::Property::Type>,
                       size_t, size_t, Callback>>());
    for (const auto& property : element.properties) {
      const auto& numbered_property =
          numbered_callbacks.at(element.name).at(property.name);
      auto& row = std::get<2>(ordered_callbacks.back());
      row.emplace_back(
          property.name, property.list_type, std::get<0>(numbered_property),
          std::get<1>(numbered_property), std::get<2>(numbered_property));
    }
  }

  Context context;
  std::get<std::string_view>(context) = header->line_ending;

  std::vector<
      std::pair<uintmax_t, std::vector<std::unique_ptr<PropertyReaderBase>>>>
      wrapped_callbacks;
  switch (header->format) {
    case PlyHeader::ASCII:
      wrapped_callbacks = BuildPropertyReaders<true, std::endian::native>(
          input, *this, ordered_callbacks, context);
      break;
    case PlyHeader::BINARY_BIG_ENDIAN:
      wrapped_callbacks = BuildPropertyReaders<false, std::endian::big>(
          input, *this, ordered_callbacks, context);
      break;
    case PlyHeader::BINARY_LITTLE_ENDIAN:
      wrapped_callbacks = BuildPropertyReaders<false, std::endian::little>(
          input, *this, ordered_callbacks, context);
      break;
  }

  for (const auto& element : wrapped_callbacks) {
    for (uintmax_t instance = 0; instance < element.first; instance++) {
      for (const auto& property_reader : element.second) {
        auto result = property_reader->Read(instance);
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
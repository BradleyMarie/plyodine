#include "plyodine/ply_reader.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <ios>
#include <istream>
#include <map>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_header_reader.h"

namespace {

enum class ErrorCode : int {
  MIN_VALUE = 1,
  BAD_STREAM = 1,
  UNEXPECTED_EOF = 2,
  CONTAINS_MISMATCHED_LINE_ENDINGS = 3,
  CONTAINS_INVALID_CHARACTER = 4,
  NEGATIVE_LIST_SIZE = 5,
  ELEMENT_TOO_FEW_TOKENS = 6,
  ELEMENT_CONTAINS_EXTRA_WHITESPACE = 7,
  ELEMENT_CONTAINS_EXTRA_TOKENS = 8,
  ELEMENT_LIST_SIZE_OUT_OF_RANGE = 9,
  ELEMENT_PROPERTY_OUT_OF_RANGE = 10,
  ELEMENT_LIST_SIZE_PARSING_FAILED = 11,
  ELEMENT_PROPERTY_PARSING_FAILED = 12,
  MAX_VALUE = 12,
};

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
  std::error_condition default_error_condition(int value) const noexcept;
} kErrorCategory;

const char* ErrorCategory::name() const noexcept {
  return "plyodine::PlyReader";
}

std::string ErrorCategory::message(int condition) const {
  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "Input stream must be in good state";
    case ErrorCode::UNEXPECTED_EOF:
      return "Unexpected EOF";
    case ErrorCode::CONTAINS_MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::CONTAINS_INVALID_CHARACTER:
      return "The input contained an invalid character";
    case ErrorCode::NEGATIVE_LIST_SIZE:
      return "The input contained a property list with a negative size";
    case ErrorCode::ELEMENT_TOO_FEW_TOKENS:
      return "The input contained an element with too few tokens";
    case ErrorCode::ELEMENT_CONTAINS_EXTRA_WHITESPACE:
      return "Non-comment ASCII lines may only contain a single space between "
             "tokens";
    case ErrorCode::ELEMENT_CONTAINS_EXTRA_TOKENS:
      return "The input contained an element with unused tokens";
    case ErrorCode::ELEMENT_LIST_SIZE_OUT_OF_RANGE:
      return "The input contained a property list size that was out of range";
    case ErrorCode::ELEMENT_PROPERTY_OUT_OF_RANGE:
      return "The input contained a property entry that was out of range";
    case ErrorCode::ELEMENT_LIST_SIZE_PARSING_FAILED:
      return "The input contained a property list size that failed to parse";
    case ErrorCode::ELEMENT_PROPERTY_PARSING_FAILED:
      return "The input contained a property entry that failed to parse";
  };

  return "Unknown Error";
}

std::error_condition ErrorCategory::default_error_condition(
    int value) const noexcept {
  if (value < static_cast<int>(ErrorCode::MIN_VALUE) ||
      value > static_cast<int>(ErrorCode::MAX_VALUE)) {
    return std::error_condition(value, *this);
  }

  return std::make_error_condition(std::errc::invalid_argument);
}

std::error_code make_error_code(ErrorCode code) {
  return std::error_code(static_cast<int>(code), kErrorCategory);
}

}  // namespace

namespace std {

template <>
struct is_error_code_enum<ErrorCode> : true_type {};

}  // namespace std

namespace plyodine {
namespace {

typedef std::tuple<std::vector<int8_t>, std::vector<uint8_t>,
                   std::vector<int16_t>, std::vector<uint16_t>,
                   std::vector<int32_t>, std::vector<uint32_t>,
                   std::vector<float>, std::vector<double>, std::string_view,
                   std::stringstream, std::string, bool>
    Context;

std::error_code ReadNextLine(std::istream& input, Context& context) {
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
          return ErrorCode::CONTAINS_MISMATCHED_LINE_ENDINGS;
        }
        line_ending.remove_prefix(1);
      }

      break;
    }

    if (c != ' ' && !std::isgraph(c)) {
      return ErrorCode::CONTAINS_INVALID_CHARACTER;
    }

    line.put(c);
  }

  if (input.fail() && !input.eof()) {
    return std::io_errc::stream;
  }

  std::get<bool>(context) = input.eof();

  return std::error_code();
}

std::error_code ReadNextToken(Context& context) {
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
      return ErrorCode::UNEXPECTED_EOF;
    }

    return ErrorCode::ELEMENT_TOO_FEW_TOKENS;
  }

  if (token.empty()) {
    return ErrorCode::ELEMENT_CONTAINS_EXTRA_WHITESPACE;
  }

  return std::error_code();
}

std::error_code CheckForUnusedTokens(Context& context) {
  auto& line = std::get<std::stringstream>(context);

  char c;
  while (line.get(c)) {
    if (std::isgraph(c)) {
      return ErrorCode::ELEMENT_CONTAINS_EXTRA_TOKENS;
    }
  }

  return std::error_code();
}

template <std::integral SizeType, bool IsListSize, typename T>
std::expected<T, std::error_code> DeserializeASCII(std::istream& input,
                                                   Context& context) {
  if (std::error_code error = ReadNextToken(context); error) {
    return std::unexpected(error);
  }

  auto& token = std::get<std::string>(context);

  T value;
  auto parsing_result =
      std::from_chars(token.data(), token.data() + token.size(), value);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    if constexpr (IsListSize) {
      return std::unexpected(ErrorCode::ELEMENT_LIST_SIZE_OUT_OF_RANGE);
    }
    return std::unexpected(ErrorCode::ELEMENT_PROPERTY_OUT_OF_RANGE);
  } else if (parsing_result.ec != std::errc{}) {
    if constexpr (IsListSize) {
      return std::unexpected(ErrorCode::ELEMENT_LIST_SIZE_PARSING_FAILED);
    }
    return std::unexpected(ErrorCode::ELEMENT_PROPERTY_PARSING_FAILED);
  }

  return value;
}

template <std::integral SizeType, bool IsListSize, typename T>
  requires std::is_class<T>::value
std::expected<T, std::error_code> DeserializeASCII(std::istream& input,
                                                   Context& context) {
  auto list_size = DeserializeASCII<SizeType, true, SizeType>(input, context);
  if (!list_size) {
    return std::unexpected(list_size.error());
  }

  if constexpr (std::is_signed<SizeType>::value) {
    if (*list_size < 0) {
      return std::unexpected(ErrorCode::NEGATIVE_LIST_SIZE);
    }
  }

  auto& result = std::get<std::vector<typename T::value_type>>(context);
  result.clear();

  for (SizeType i = 0; i < *list_size; i++) {
    auto entry = DeserializeASCII<SizeType, false, typename T::value_type>(
        input, context);
    if (!entry) {
      return std::unexpected(entry.error());
    }

    result.push_back(*entry);
  }

  return result;
}

template <std::endian Endianness, std::integral SizeType, std::integral T>
std::expected<T, std::error_code> DeserializeBinary(std::istream& input,
                                                    Context& context) {
  T result;
  input.read(reinterpret_cast<char*>(&result), sizeof(T));
  if (input.fail()) {
    if (input.eof()) {
      return std::unexpected(ErrorCode::UNEXPECTED_EOF);
    }
    return std::unexpected(std::io_errc::stream);
  }

  if (Endianness != std::endian::native) {
    result = std::byteswap(result);
  }

  return result;
}

template <std::endian Endianness, std::integral SizeType, std::floating_point T>
std::expected<T, std::error_code> DeserializeBinary(std::istream& input,
                                                    Context& context) {
  std::conditional_t<std::is_same<T, float>::value, uint32_t, uint64_t> read;

  input.read(reinterpret_cast<char*>(&read), sizeof(read));
  if (input.fail()) {
    if (input.eof()) {
      return std::unexpected(ErrorCode::UNEXPECTED_EOF);
    }
    return std::unexpected(std::io_errc::stream);
  }

  if (Endianness != std::endian::native) {
    read = std::byteswap(read);
  }

  return std::bit_cast<T>(read);
}

template <std::endian Endianness, std::integral SizeType, typename T>
  requires std::is_class<T>::value
std::expected<T, std::error_code> DeserializeBinary(std::istream& input,
                                                    Context& context) {
  auto list_size =
      DeserializeBinary<Endianness, SizeType, SizeType>(input, context);
  if (!list_size) {
    return std::unexpected(list_size.error());
  }

  if constexpr (std::is_signed<SizeType>::value) {
    if (*list_size < 0) {
      return std::unexpected(ErrorCode::NEGATIVE_LIST_SIZE);
    }
  }

  auto& result = std::get<std::vector<typename T::value_type>>(context);
  result.clear();

  for (SizeType i = 0; i < *list_size; i++) {
    auto entry =
        DeserializeBinary<Endianness, SizeType, typename T::value_type>(
            input, context);
    if (!entry) {
      return std::unexpected(entry.error());
    }

    result.push_back(*entry);
  }

  return result;
}

class PropertyReaderBase {
 public:
  virtual std::error_code Read(uintmax_t instance) = 0;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
class PropertyReader final : public PropertyReaderBase {
 public:
  PropertyReader(std::istream& input, PlyReader& ply_reader,
                 std::error_code (PlyReader::*callback)(const std::string&,
                                                        size_t,
                                                        const std::string&,
                                                        size_t, uintmax_t, T),
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

  std::error_code Read(uintmax_t instance) override;

 private:
  std::istream& input_;
  PlyReader& ply_reader_;
  std::error_code (PlyReader::*callback_)(const std::string&, size_t,
                                          const std::string&, size_t, uintmax_t,
                                          T);
  const std::string& element_name_;
  size_t element_index_;
  const std::string& property_name_;
  size_t property_index_;
  size_t actual_property_index_;
  size_t num_properties_;
  Context& context_;
};

template <bool Ascii, std::endian Endianness, typename SizeType, typename T>
std::error_code PropertyReader<Ascii, Endianness, SizeType, T>::Read(
    uintmax_t instance) {
  if constexpr (Ascii) {
    if (actual_property_index_ == 0) {
      if (std::error_code error = ReadNextLine(input_, context_); error) {
        return error;
      }
    }
  }

  std::expected<T, std::error_code> result;
  if constexpr (Ascii) {
    result = DeserializeASCII<SizeType, false, T>(input_, context_);
  } else {
    result = DeserializeBinary<Endianness, SizeType, T>(input_, context_);
  }

  if (!result) {
    return result.error();
  }

  if (callback_ == nullptr) {
    return std::error_code();
  }

  if (std::error_code error = (ply_reader_.*callback_)(
          element_name_, element_index_, property_name_, property_index_,
          instance, *result);
      error) {
    return error;
  }

  if constexpr (Ascii) {
    if (actual_property_index_ + 1 == num_properties_) {
      if (std::error_code error = CheckForUnusedTokens(context_); error) {
        return error;
      }
    }
  }

  return std::error_code();
}

template <bool Ascii, std::endian Endianness, typename T>
std::unique_ptr<PropertyReaderBase> BuildPropertyReader(
    std::istream& input, PlyReader& ply_reader,
    std::error_code (PlyReader::*callback)(const std::string&, size_t,
                                           const std::string&, size_t,
                                           uintmax_t, T),
    const std::string& element_name, size_t element_index,
    const std::string& property_name, size_t property_index,
    size_t actual_property_index, size_t num_properties,
    std::optional<PlyHeader::Property::Type> list_type, Context& context) {
  if constexpr (std::is_class<T>::value) {
    if (list_type.has_value()) {
      switch (*list_type) {
        case PlyHeader::Property::CHAR:
          return std::make_unique<PropertyReader<Ascii, Endianness, int8_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UCHAR:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, uint8_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::SHORT:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, int16_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::USHORT:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, uint16_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::INT:
          return std::make_unique<
              PropertyReader<Ascii, Endianness, int32_t, T>>(
              input, ply_reader, callback, element_name, element_index,
              property_name, property_index, actual_property_index,
              num_properties, context);
        case PlyHeader::Property::UINT:
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
    const auto& element = ordered_callbacks[actual_element_index];
    const auto& properties = std::get<2>(element);

    result.emplace_back(std::get<uintmax_t>(element),
                        std::vector<std::unique_ptr<PropertyReaderBase>>());
    for (size_t actual_property_index = 0;
         actual_property_index < properties.size(); actual_property_index++) {
      const auto& property = properties[actual_property_index];

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

std::error_code PlyReader::ReadFrom(std::istream& input) {
  if (!input) {
    return ErrorCode::BAD_STREAM;
  }

  auto header = ReadPlyHeader(input);
  if (!header) {
    return header.error();
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
          case PlyHeader::Property::CHAR:
            callback = Int8PropertyListCallback();
            break;
          case PlyHeader::Property::UCHAR:
            callback = UInt8PropertyListCallback();
            break;
          case PlyHeader::Property::SHORT:
            callback = Int16PropertyListCallback();
            break;
          case PlyHeader::Property::USHORT:
            callback = UInt16PropertyListCallback();
            break;
          case PlyHeader::Property::INT:
            callback = Int32PropertyListCallback();
            break;
          case PlyHeader::Property::UINT:
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
          case PlyHeader::Property::CHAR:
            callback = Int8PropertyCallback();
            break;
          case PlyHeader::Property::UCHAR:
            callback = UInt8PropertyCallback();
            break;
          case PlyHeader::Property::SHORT:
            callback = Int16PropertyCallback();
            break;
          case PlyHeader::Property::USHORT:
            callback = UInt16PropertyCallback();
            break;
          case PlyHeader::Property::INT:
            callback = Int32PropertyCallback();
            break;
          case PlyHeader::Property::UINT:
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
  if (std::error_code error = Start(num_element_instances, callbacks,
                                    header->comments, header->object_info);
      error) {
    return error;
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
      const auto& numbered_property = numbered_callbacks.find(element.name)
                                          ->second.find(property.name)
                                          ->second;
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
        if (std::error_code error = property_reader->Read(instance); error) {
          return error;
        }
      }
    }
  }

  return std::error_code();
}

// Static assertions to ensure float types are properly sized
static_assert(sizeof(double) == 8);
static_assert(sizeof(float) == 4);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
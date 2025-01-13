#include "plyodine/ply_reader.h"

#include <bit>
#include <charconv>
#include <cstdint>
#include <ios>
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
  UNKNOWN_ELEMENT = 2,
  UNKNOWN_PROPERTY = 3,
  UNSUPPORTED_CONVERSION = 4,
  UNEXPECTED_EOF = 5,
  CONTAINS_MISMATCHED_LINE_ENDINGS = 6,
  CONTAINS_INVALID_CHARACTER = 7,
  NEGATIVE_LIST_SIZE = 8,
  ELEMENT_TOO_FEW_TOKENS = 9,
  ELEMENT_CONTAINS_EXTRA_WHITESPACE = 10,
  ELEMENT_CONTAINS_EXTRA_TOKENS = 11,
  ELEMENT_LIST_SIZE_OUT_OF_RANGE = 12,
  ELEMENT_PROPERTY_OUT_OF_RANGE = 13,
  ELEMENT_LIST_SIZE_PARSING_FAILED = 14,
  ELEMENT_PROPERTY_PARSING_FAILED = 15,
  CONVERSION_SIGNED_UNDERFLOW = 16,
  CONVERSION_UNSIGNED_UNDERFLOW = 17,
  CONVERSION_INTEGER_OVERFLOW = 18,
  CONVERSION_FLOAT_UNDERFLOW = 19,
  CONVERSION_FLOAT_OVERFLOW = 20,
  MAX_VALUE = 20,
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
    case ErrorCode::UNKNOWN_ELEMENT:
      return "";
    case ErrorCode::UNKNOWN_PROPERTY:
      return "";
    case ErrorCode::UNSUPPORTED_CONVERSION:
      return "";
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
    case ErrorCode::CONVERSION_SIGNED_UNDERFLOW:
      return "";
    case ErrorCode::CONVERSION_UNSIGNED_UNDERFLOW:
      return "";
    case ErrorCode::CONVERSION_INTEGER_OVERFLOW:
      return "";
    case ErrorCode::CONVERSION_FLOAT_UNDERFLOW:
      return "";
    case ErrorCode::CONVERSION_FLOAT_OVERFLOW:
      return "";
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

using ContextData =
    std::tuple<int8_t, std::vector<int8_t>, uint8_t, std::vector<uint8_t>,
               int16_t, std::vector<int16_t>, uint16_t, std::vector<uint16_t>,
               int32_t, std::vector<int32_t>, uint32_t, std::vector<uint32_t>,
               float, std::vector<float>, double, std::vector<double>>;

struct Context final {
  ContextData data;
  std::string_view line_ending;
  std::stringstream line;
  std::string token;
};

using AppendFunc = void (*)(Context&);
using ConvertFunc = std::error_code (*)(Context&);
using Handler = std::function<std::error_code(Context&)>;
using OnConversionErrorFunc = std::function<std::error_code(
    const std::string&, const std::string&, std::error_code)>;
using PropertyCallback =
    std::variant<std::function<std::error_code(int8_t)>,
                 std::function<std::error_code(std::span<const int8_t>)>,
                 std::function<std::error_code(uint8_t)>,
                 std::function<std::error_code(std::span<const uint8_t>)>,
                 std::function<std::error_code(int16_t)>,
                 std::function<std::error_code(std::span<const int16_t>)>,
                 std::function<std::error_code(uint16_t)>,
                 std::function<std::error_code(std::span<const uint16_t>)>,
                 std::function<std::error_code(int32_t)>,
                 std::function<std::error_code(std::span<const int32_t>)>,
                 std::function<std::error_code(uint32_t)>,
                 std::function<std::error_code(std::span<const uint32_t>)>,
                 std::function<std::error_code(float)>,
                 std::function<std::error_code(std::span<const float>)>,
                 std::function<std::error_code(double)>,
                 std::function<std::error_code(std::span<const double>)>>;
using ReadFunc = std::error_code (*)(std::istream&, Context&);

std::error_code ReadNextLine(std::istream& stream, Context& context) {
  std::string_view line_ending = context.line_ending;

  context.line.str("");
  context.line.clear();

  char c = 0;
  while (stream.get(c)) {
    if (c == line_ending[0]) {
      line_ending.remove_prefix(1);

      while (!line_ending.empty() && stream.get(c)) {
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

    context.line.put(c);
  }

  if (!c) {
    return ErrorCode::UNEXPECTED_EOF;
  }

  if (stream.fail() && !stream.eof()) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

std::error_code ReadNextToken(Context& context) {
  context.token.clear();

  char c = 0;
  while (context.line.get(c)) {
    if (c == ' ') {
      break;
    }

    context.token.push_back(c);
  }

  if (!c) {
    return ErrorCode::ELEMENT_TOO_FEW_TOKENS;
  }

  if (context.token.empty()) {
    return ErrorCode::ELEMENT_CONTAINS_EXTRA_WHITESPACE;
  }

  return std::error_code();
}

template <typename T>
std::error_code ReadASCII(std::istream& input, Context& context) {
  if (std::error_code error = ReadNextToken(context); error) {
    return error;
  }

  T value;
  auto parsing_result = std::from_chars(
      context.token.data(), context.token.data() + context.token.size(), value);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return ErrorCode::ELEMENT_PROPERTY_OUT_OF_RANGE;
  } else if (parsing_result.ec != std::errc{}) {
    return ErrorCode::ELEMENT_PROPERTY_PARSING_FAILED;
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code ReadBinary(std::istream& stream, Context& context) {
  T value;
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
  if (stream.fail()) {
    if (stream.eof()) {
      return ErrorCode::UNEXPECTED_EOF;
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::floating_point T>
std::error_code ReadBinary(std::istream& stream, Context& context) {
  std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t> value;

  stream.read(reinterpret_cast<char*>(&value), sizeof(value));
  if (stream.fail()) {
    if (stream.eof()) {
      return ErrorCode::UNEXPECTED_EOF;
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  std::get<T>(context.data) = std::bit_cast<T>(value);

  return std::error_code();
}

ReadFunc GetReadFunc(PlyHeader::Format format, PlyHeader::Property::Type type) {
  static constexpr ReadFunc ascii_read_funcs[8] = {
      ReadASCII<std::tuple_element_t<0, ContextData>>,
      ReadASCII<std::tuple_element_t<2, ContextData>>,
      ReadASCII<std::tuple_element_t<4, ContextData>>,
      ReadASCII<std::tuple_element_t<6, ContextData>>,
      ReadASCII<std::tuple_element_t<8, ContextData>>,
      ReadASCII<std::tuple_element_t<10, ContextData>>,
      ReadASCII<std::tuple_element_t<12, ContextData>>,
      ReadASCII<std::tuple_element_t<14, ContextData>>,
  };

  static constexpr ReadFunc big_endian_read_funcs[8] = {
      ReadBinary<std::endian::big, std::tuple_element_t<0, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<2, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<4, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<6, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<8, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<10, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<12, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<14, ContextData>>,
  };

  static constexpr ReadFunc little_endian_read_funcs[8] = {
      ReadBinary<std::endian::little, std::tuple_element_t<0, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<2, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<4, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<6, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<8, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<10, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<12, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<14, ContextData>>,
  };

  if (format == PlyHeader::Format::ASCII) {
    return ascii_read_funcs[static_cast<size_t>(type)];
  }

  if (format == PlyHeader::Format::BINARY_BIG_ENDIAN) {
    return big_endian_read_funcs[static_cast<size_t>(type)];
  }

  return little_endian_read_funcs[static_cast<size_t>(type)];
}

template <typename Source, typename Dest>
std::error_code Convert(Context& context) {
  if constexpr (std::is_signed_v<Source> && !std::is_signed_v<Dest>) {
    if (std::get<Source>(context.data) < 0) {
      return ErrorCode::CONVERSION_UNSIGNED_UNDERFLOW;
    }
  }

  if constexpr (sizeof(Source) > sizeof(Dest)) {
    if constexpr (std::is_signed_v<Source> && std::is_signed_v<Dest>) {
      if (std::get<Source>(context.data) < std::numeric_limits<Dest>::min()) {
        return std::is_floating_point_v<Dest>
                   ? ErrorCode::CONVERSION_SIGNED_UNDERFLOW
                   : ErrorCode::CONVERSION_FLOAT_UNDERFLOW;
      }
    }

    if (std::get<Source>(context.data) > std::numeric_limits<Dest>::max()) {
      return std::is_floating_point_v<Dest>
                 ? ErrorCode::CONVERSION_INTEGER_OVERFLOW
                 : ErrorCode::CONVERSION_FLOAT_OVERFLOW;
    }
  }

  std::get<Dest>(context.data) =
      static_cast<Dest>(std::get<Source>(context.data));

  return std::error_code();
}

template <size_t SourceTypeIndex, size_t DestTypeIndex>
constexpr ConvertFunc GetConvertFunc() {
  if constexpr (SourceTypeIndex >= 6 && DestTypeIndex < 6) {
    return nullptr;
  }

  return Convert<std::tuple_element_t<SourceTypeIndex * 2, ContextData>,
                 std::tuple_element_t<DestTypeIndex * 2, ContextData>>;
}

template <size_t SourceTypeIndex>
constexpr std::array<ConvertFunc, 8> GetConvertFuncs() {
  return {
      GetConvertFunc<SourceTypeIndex, 0>(),
      GetConvertFunc<SourceTypeIndex, 1>(),
      GetConvertFunc<SourceTypeIndex, 2>(),
      GetConvertFunc<SourceTypeIndex, 3>(),
      GetConvertFunc<SourceTypeIndex, 4>(),
      GetConvertFunc<SourceTypeIndex, 5>(),
      GetConvertFunc<SourceTypeIndex, 6>(),
      GetConvertFunc<SourceTypeIndex, 7>(),
  };
}

ConvertFunc GetConvertFunc(PlyHeader::Property::Type source,
                           PlyHeader::Property::Type dest) {
  static constexpr std::array<ConvertFunc, 8> conversion_funcs[8] = {
      GetConvertFuncs<0>(), GetConvertFuncs<1>(), GetConvertFuncs<2>(),
      GetConvertFuncs<3>(), GetConvertFuncs<4>(), GetConvertFuncs<5>(),
      GetConvertFuncs<6>(), GetConvertFuncs<7>(),
  };

  return conversion_funcs[static_cast<size_t>(source)]
                         [static_cast<size_t>(dest)];
}

template <size_t Index>
void Append(Context& context) {
  std::get<2 * Index + 1>(context.data)
      .push_back(std::get<2 * Index>(context.data));
}

AppendFunc GetAppendFunc(PlyHeader::Property::Type dest_type) {
  static constexpr AppendFunc append_funcs[8] = {
      Append<0>, Append<1>, Append<2>, Append<3>,
      Append<4>, Append<5>, Append<6>, Append<7>,
  };

  return append_funcs[static_cast<size_t>(dest_type)];
}

template <typename T>
Handler MakeHandler(const std::function<std::error_code(T)>& callback) {
  return [&](Context& context) { return callback(std::get<T>(context.data)); };
}

template <typename T>
Handler MakeHandler(
    const std::function<std::error_code(std::span<const T>)>& callback) {
  return [&](Context& context) {
    auto& data = std::get<std::vector<T>>(context.data);
    std::error_code result = callback(data);
    data.clear();
    return result;
  };
}

Handler MakeHandler(const PropertyCallback& callback) {
  return std::visit(
      [](const auto& true_callback) -> Handler {
        if (!true_callback) {
          return Handler();
        }

        return MakeHandler(true_callback);
      },
      callback);
}

class PropertyParser {
 public:
  PropertyParser(PlyHeader::Format format,
                 std::optional<PlyHeader::Property::Type> list_type,
                 PlyHeader::Property::Type source_type,
                 PlyHeader::Property::Type dest_type, Handler handler,
                 OnConversionErrorFunc on_conversion_error,
                 const std::string& element_name,
                 const std::string& property_name);

  std::error_code Parse(std::istream& stream, Context& context) const;

 private:
  const std::string& element_name_;
  const std::string& property_name_;
  ReadFunc read_length_;
  ConvertFunc convert_length_;
  ReadFunc read_;
  ConvertFunc convert_;
  AppendFunc append_to_list_;
  OnConversionErrorFunc on_conversion_error_;
  Handler handler_;
};

PropertyParser::PropertyParser(
    PlyHeader::Format format,
    std::optional<PlyHeader::Property::Type> list_type,
    PlyHeader::Property::Type source_type, PlyHeader::Property::Type dest_type,
    Handler handler, OnConversionErrorFunc on_conversion_error,
    const std::string& element_name, const std::string& property_name)
    : element_name_(element_name),
      property_name_(property_name),
      read_length_(list_type ? GetReadFunc(format, *list_type) : nullptr),
      convert_length_(
          list_type
              ? GetConvertFunc(*list_type, PlyHeader::Property::Type::UINT)
              : nullptr),
      read_(GetReadFunc(format, source_type)),
      convert_(GetConvertFunc(source_type, dest_type)),
      append_to_list_(list_type ? GetAppendFunc(dest_type) : nullptr),
      on_conversion_error_(std::move(on_conversion_error)),
      handler_(std::move(handler)) {}

std::error_code PropertyParser::Parse(std::istream& stream,
                                      Context& context) const {
  uint32_t length = 1;
  if (read_length_) {
    if (std::error_code error = read_length_(stream, context); error) {
      if (error == ErrorCode::ELEMENT_PROPERTY_OUT_OF_RANGE) {
        return ErrorCode::ELEMENT_LIST_SIZE_OUT_OF_RANGE;
      }

      if (error == ErrorCode::ELEMENT_PROPERTY_PARSING_FAILED) {
        return ErrorCode::ELEMENT_LIST_SIZE_PARSING_FAILED;
      }

      return error;
    }

    if (convert_length_(context)) {
      return ErrorCode::NEGATIVE_LIST_SIZE;
    }

    length = std::get<uint32_t>(context.data);
  }

  for (uint32_t i = 0; i < length; i++) {
    if (std::error_code error = read_(stream, context); error) {
      return error;
    }

    if (std::error_code error = convert_(context); error) {
      return on_conversion_error_(element_name_, property_name_, error);
    }

    if (append_to_list_) {
      append_to_list_(context);
    }
  }

  if (handler_) {
    if (std::error_code error = handler_(context); error) {
      return error;
    }
  }

  return std::error_code();
}

PropertyCallback MakeEmptyCallback(PlyHeader::Property::Type data_type,
                                   bool is_list) {
  static const PropertyCallback empty_callbacks[16] = {
      PropertyCallback(std::in_place_index<0>),
      PropertyCallback(std::in_place_index<1>),
      PropertyCallback(std::in_place_index<2>),
      PropertyCallback(std::in_place_index<3>),
      PropertyCallback(std::in_place_index<4>),
      PropertyCallback(std::in_place_index<5>),
      PropertyCallback(std::in_place_index<6>),
      PropertyCallback(std::in_place_index<7>),
      PropertyCallback(std::in_place_index<8>),
      PropertyCallback(std::in_place_index<9>),
      PropertyCallback(std::in_place_index<10>),
      PropertyCallback(std::in_place_index<11>),
      PropertyCallback(std::in_place_index<12>),
      PropertyCallback(std::in_place_index<13>),
      PropertyCallback(std::in_place_index<14>),
      PropertyCallback(std::in_place_index<15>)};

  return empty_callbacks[2 * static_cast<size_t>(data_type) +
                         static_cast<size_t>(is_list)];
}

}  // namespace

std::error_code PlyReader::ReadFrom(std::istream& stream) {
  if (!stream) {
    return ErrorCode::BAD_STREAM;
  }

  auto header = ReadPlyHeader(stream);
  if (!header) {
    return header.error();
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, PropertyCallback>>
      actual_callbacks;
  for (const auto& element : header->elements) {
    num_element_instances[element.name] = element.num_in_file;

    auto insertion_iterator =
        actual_callbacks
            .emplace(element.name, std::map<std::string, PropertyCallback>())
            .first;
    for (const auto& property : element.properties) {
      insertion_iterator->second[property.name] =
          MakeEmptyCallback(property.data_type, property.list_type.has_value());
    }
  }

  auto requested_callbacks = actual_callbacks;
  if (std::error_code error =
          Start(std::move(num_element_instances), requested_callbacks,
                std::move(header->comments), std::move(header->object_info));
      error) {
    return error;
  }

  for (auto& [element_name, element_callbacks] : requested_callbacks) {
    auto element_iter = actual_callbacks.find(element_name);
    if (element_iter == actual_callbacks.end()) {
      return ErrorCode::UNKNOWN_ELEMENT;
    }

    for (auto& [property_name, property_callback] : element_callbacks) {
      auto property_iter = element_iter->second.find(property_name);
      if (property_iter == element_iter->second.end()) {
        return ErrorCode::UNKNOWN_PROPERTY;
      }

      size_t original_index = property_iter->second.index();
      size_t desired_index = property_callback.index();

      if ((original_index & 0x1u) != (desired_index & 0x1u)) {
        return ErrorCode::UNSUPPORTED_CONVERSION;
      }

      if ((original_index >> 1u) < 6 && (desired_index >> 1u) >= 6) {
        return ErrorCode::UNSUPPORTED_CONVERSION;
      }

      property_iter->second = std::move(property_callback);
    }
  }

  std::vector<std::vector<PropertyParser>> parsers;
  for (const PlyHeader::Element& element : header->elements) {
    parsers.emplace_back();
    for (const PlyHeader::Property& property : element.properties) {
      const PropertyCallback& callback = actual_callbacks.find(element.name)
                                             ->second.find(property.name)
                                             ->second;
      parsers.back().emplace_back(
          header->format, property.list_type, property.data_type,
          static_cast<PlyHeader::Property::Type>(callback.index() >> 1u),
          MakeHandler(callback),
          [&, this](const std::string& element_name,
                    const std::string& property_name,
                    std::error_code original_error) -> std::error_code {
            ConversionFailureReason reason;
            if (original_error == ErrorCode::CONVERSION_SIGNED_UNDERFLOW) {
              reason = ConversionFailureReason::SIGNED_INTEGER_UNDERFLOW;
            } else if (original_error ==
                       ErrorCode::CONVERSION_UNSIGNED_UNDERFLOW) {
              reason = ConversionFailureReason::UNSIGNED_INTEGER_UNDERFLOW;
            } else if (original_error ==
                       ErrorCode::CONVERSION_INTEGER_OVERFLOW) {
              reason = ConversionFailureReason::INTEGER_OVERFLOW;
            } else if (original_error ==
                       ErrorCode::CONVERSION_FLOAT_UNDERFLOW) {
              reason = ConversionFailureReason::FLOAT_UNDERFLOW;
            } else {
              reason = ConversionFailureReason::FLOAT_OVERFLOW;
            }

            if (std::error_code error =
                    OnConversionError(element_name, property_name, reason);
                error) {
              return error;
            }

            return original_error;
          },
          element.name, property.name);
    }
  }

  Context context;
  context.line_ending = header->line_ending;
  for (size_t element_index = 0; element_index < header->elements.size();
       element_index++) {
    for (size_t instance = 0;
         instance < header->elements[element_index].num_in_file; instance++) {
      if (header->format == PlyHeader::Format::ASCII) {
        if (std::error_code error = ReadNextLine(stream, context); error) {
          return error;
        }
      }

      for (size_t property_index = 0;
           property_index < header->elements[element_index].properties.size();
           property_index++) {
        if (std::error_code error =
                parsers[element_index][property_index].Parse(stream, context);
            error) {
          return error;
        }
      }

      if (header->format == PlyHeader::Format::ASCII) {
        if (std::error_code error = ReadNextToken(context);
            error != ErrorCode::ELEMENT_TOO_FEW_TOKENS) {
          if (!error) {
            return ErrorCode::ELEMENT_CONTAINS_EXTRA_TOKENS;
          }

          return error;
        }
      }
    }
  }

  return std::error_code();
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8);
static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
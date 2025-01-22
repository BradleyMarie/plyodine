#include "plyodine/ply_reader.h"

#include <array>
#include <bit>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <format>
#include <ios>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_header_reader.h"

namespace plyodine {
namespace {

enum class EntryType : uint8_t {
  LIST_SIZE = 0,
  LIST_VALUE = 1,
  VALUE = 2,
};

enum class ErrorType : uint8_t {
  BAD_STREAM = 0,
  INVALID_CONVERSION = 1,
  UNEXPECTED_EOF = 2,
  UNEXPECTED_EOF_NO_PROPERTIES = 3,
  MISMATCHED_LINE_ENDINGS = 4,
  INVALID_CHARACTERS = 5,
  MISSING_TOKEN = 6,
  UNUSED_TOKEN = 7,
  FAILED_TO_PARSE = 8,
  OUT_OF_RANGE = 9,
  OVERFLOW = 10,
  UNDERFLOW = 11,
  MAX_VALUE = 11,
};

bool IsInvalidConversion(size_t source, size_t dest) {
  return (source & 0x1u) != (dest & 0x1u) ||
         ((source >> 1u) < 6 && (dest >> 1u) >= 6) ||
         ((source >> 1u) >= 6 && (dest >> 1u) < 6);
}

bool IsInvalidConversion(uint8_t payload) {
  uint8_t source = payload & 0xFu;
  uint8_t dest = (payload >> 4u);
  return IsInvalidConversion(source, dest);
}

bool IsUnexpectedEof(uint8_t payload) {
  if (payload >= 48) {
    return false;
  }

  EntryType entry_type = static_cast<EntryType>(payload >> 3u);
  PlyHeader::Property::Type data_type =
      static_cast<PlyHeader::Property::Type>(payload & 0x7u);

  return entry_type != EntryType::LIST_SIZE ||
         (data_type != PlyHeader::Property::Type::FLOAT &&
          data_type != PlyHeader::Property::Type::DOUBLE);
}

bool IsMissingToken(uint8_t payload) { return IsUnexpectedEof(payload); }
bool IsFailedToParse(uint8_t payload) { return IsUnexpectedEof(payload); }
bool IsOutOfRange(uint8_t payload) { return IsUnexpectedEof(payload); }

std::optional<
    std::tuple<PlyHeader::Property::Type, PlyHeader::Property::Type, bool>>
DecodeOverUnderFlowPayload(uint8_t payload) {
  if (payload >= 128) {
    return std::nullopt;
  }
  return std::make_tuple(
      static_cast<PlyHeader::Property::Type>((payload >> 1u) & 0x7u),
      static_cast<PlyHeader::Property::Type>((payload >> 4u) & 0x7u),
      static_cast<bool>(payload & 1u));
}

bool IsOverflow(uint8_t payload) {
  auto decoded = DecodeOverUnderFlowPayload(payload);
  if (!decoded) {
    return false;
  }

  switch (auto [source, dest, _] = *decoded; dest) {
    case PlyHeader::Property::Type::CHAR:
      return source == PlyHeader::Property::Type::UCHAR ||
             source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::USHORT ||
             source == PlyHeader::Property::Type::INT ||
             source == PlyHeader::Property::Type::UINT;
    case PlyHeader::Property::Type::UCHAR:
      return source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::USHORT ||
             source == PlyHeader::Property::Type::INT ||
             source == PlyHeader::Property::Type::UINT;
    case PlyHeader::Property::Type::SHORT:
      return source == PlyHeader::Property::Type::USHORT ||
             source == PlyHeader::Property::Type::INT ||
             source == PlyHeader::Property::Type::UINT;
    case PlyHeader::Property::Type::USHORT:
      return source == PlyHeader::Property::Type::INT ||
             source == PlyHeader::Property::Type::UINT;
    case PlyHeader::Property::Type::INT:
      return source == PlyHeader::Property::Type::UINT;
    case PlyHeader::Property::Type::FLOAT:
      return source == PlyHeader::Property::Type::DOUBLE;
    default:
      break;
  }

  return false;
}

bool IsUnderflow(uint8_t payload) {
  auto decoded = DecodeOverUnderFlowPayload(payload);
  if (!decoded) {
    return false;
  }

  switch (auto [source, dest, _] = *decoded; dest) {
    case PlyHeader::Property::Type::CHAR:
      return source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::INT;
    case PlyHeader::Property::Type::UCHAR:
      return source == PlyHeader::Property::Type::CHAR ||
             source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::INT;
    case PlyHeader::Property::Type::SHORT:
      return source == PlyHeader::Property::Type::INT;
    case PlyHeader::Property::Type::USHORT:
      return source == PlyHeader::Property::Type::CHAR ||
             source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::INT;
    case PlyHeader::Property::Type::UINT:
      return source == PlyHeader::Property::Type::CHAR ||
             source == PlyHeader::Property::Type::SHORT ||
             source == PlyHeader::Property::Type::INT;
    case PlyHeader::Property::Type::FLOAT:
      return source == PlyHeader::Property::Type::DOUBLE;
    default:
      break;
  }

  return false;
}

std::optional<std::pair<ErrorType, uint8_t>> DecodeError(int error) {
  if (error <= 0) {
    return std::nullopt;
  }

  error -= 1;

  if (error > std::numeric_limits<uint16_t>::max()) {
    return std::nullopt;
  }

  ErrorType type = static_cast<ErrorType>(error & 0xFF);
  uint8_t payload = static_cast<uint8_t>((error >> 8) & 0xFF);
  switch (type) {
    case ErrorType::BAD_STREAM:
      if (payload != 0) {
        return std::nullopt;
      }
      break;
    case ErrorType::INVALID_CONVERSION:
      if (!IsInvalidConversion(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::UNEXPECTED_EOF:
      if (!IsUnexpectedEof(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::UNEXPECTED_EOF_NO_PROPERTIES:
      if (payload != 0) {
        return std::nullopt;
      }
      break;
    case ErrorType::MISMATCHED_LINE_ENDINGS:
      if (payload != 0) {
        return std::nullopt;
      }
      break;
    case ErrorType::INVALID_CHARACTERS:
      if (payload != 0) {
        return std::nullopt;
      }
      break;
    case ErrorType::MISSING_TOKEN:
      if (!IsMissingToken(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::UNUSED_TOKEN:
      if (payload != 0) {
        return std::nullopt;
      }
      break;
    case ErrorType::FAILED_TO_PARSE:
      if (!IsFailedToParse(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::OUT_OF_RANGE:
      if (!IsOutOfRange(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::OVERFLOW:
      if (!IsOverflow(payload)) {
        return std::nullopt;
      }
      break;
    case ErrorType::UNDERFLOW:
      if (!IsUnderflow(payload)) {
        return std::nullopt;
      }
      break;
    default:
      return std::nullopt;
  }

  return std::make_pair(type, payload);
}

int EncodeError(ErrorType type, uint8_t payload) {
  int type_as_int = static_cast<int>(type);
  int payload_as_int = static_cast<int>(payload);
  return (type_as_int | (payload_as_int << 8)) + 1;
}

std::string InvalidConversionMessage(uint8_t payload) {
  static constexpr std::string_view type_names[16] = {
      "'char' property",   "'char' property list",
      "'uchar' property",  "'uchar' property list",
      "'short' property",  "'short' property list",
      "'ushort' property", "'ushort' property list",
      "'int' property",    "'int' property list",
      "'uint' property",   "'uint' property list",
      "'float' property",  "'float' property list",
      "'double' property", "'double' property list"};

  uint8_t source = payload & 0xF;
  uint8_t dest = (payload >> 4u) & 0xF;

  std::string message = "A callback requested an unsupported conversion from ";
  message += type_names[source];
  message += " to ";
  message += type_names[dest];

  return message;
}

std::string MissingUnexpectedEofMessage(std::string_view prefix,
                                        uint8_t payload) {
  static constexpr std::string_view value_type[3] = {
      "the length of a ",
      "an entry of a ",
      "the value of a ",
  };

  static constexpr std::string_view type_names[3][8] = {
      {"property list with size type 'char'",
       "property list with size type 'uchar'",
       "property list with size type 'short'",
       "property list with size type 'ushort'",
       "property list with size type 'int'",
       "property list with size type 'uint'", "INVALID", "INVALID"},
      {"property list with data type 'char'",
       "property list with data type 'uchar'",
       "property list with data type 'short'",
       "property list with data type 'ushort'",
       "property list with data type 'int'",
       "property list with data type 'uint'",
       "property list with data type 'float'",
       "property list with data type 'double'"},
      {"property with type 'char'", "property with type 'uchar'",
       "property with type 'short'", "property with type 'ushort'",
       "property with type 'int'", "property with type 'uint'",
       "property with type 'float'", "property with type 'double'"}};

  uint8_t entry_type = payload >> 3u;
  uint8_t data_type = payload & 0x7u;

  std::string result(prefix);
  result += value_type[entry_type];
  result += type_names[entry_type][data_type];
  result += ")";

  return result;
}

std::string FailedToParseMessage(uint8_t payload) {
  static constexpr std::string_view prefix[3] = {
      "The input contained a property list with size type '",
      "The input contained a property list with data type '",
      "The input contained a property with type '",
  };

  static constexpr std::string_view type[8] = {
      "char", "uchar", "short", "ushort", "int", "uint", "float", "double",
  };

  static constexpr std::string_view value_type[3] = {
      "a length",
      "an entry",
      "a value",
  };

  uint8_t entry_type = payload >> 3u;
  uint8_t data_type = payload & 0x7u;

  std::string result(prefix[entry_type]);
  result += type[data_type];
  result += "' that had ";
  result += value_type[entry_type];
  result += " could not be parsed";

  return result;
}

std::string OutOfRangeMessage(uint8_t payload) {
  static constexpr std::string_view prefix[3] = {
      "The input contained a property list with size type '",
      "The input contained a property list with data type '",
      "The input contained a property with type '",
  };

  static constexpr std::string_view type[8] = {
      "char", "uchar", "short", "ushort", "int", "uint", "float", "double",
  };

  static constexpr std::string_view value_type[3] = {
      "a length",
      "an entry",
      "a value",
  };

  static const std::string data_type_min[3][8]{
      {"0", "0", "0", "0", "0", "0", "INVALID", "INVALID"},
      {"-128", "0", "-32,767", "0", "-2,147,483,647", "0",
       std::format("~{}", std::numeric_limits<float>::lowest()),
       std::format("~{}", std::numeric_limits<double>::lowest())},
      {"-128", "0", "-32,767", "0", "-2,147,483,647", "0",
       std::format("~{}", std::numeric_limits<float>::lowest()),
       std::format("~{}", std::numeric_limits<double>::lowest())}};

  static const std::string data_type_max[8] = {
      "127",
      "255",
      "32,767",
      "65,535",
      "2,147,483,647",
      "4,294,967,295",
      std::format("~{}", std::numeric_limits<float>::max()),
      std::format("~{}", std::numeric_limits<double>::max())};

  uint8_t entry_type = payload >> 3u;
  uint8_t data_type = payload & 0x7u;

  std::string result(prefix[entry_type]);
  result += type[data_type];
  result += "' that had ";
  result += value_type[entry_type];
  result += " that was out of range (must be between ";
  result += data_type_min[entry_type][data_type];
  result += " and ";
  result += data_type_max[data_type];
  result += ")";

  return result;
}

std::string OverflowedUnderflowedMessage(std::string_view type,
                                         uint8_t payload) {
  static constexpr std::string_view prefix[2] = {
      "The input contained a property with type '",
      "The input contained a property list with data type '"};

  static constexpr std::string_view types[8] = {
      "char", "uchar", "short", "ushort", "int", "uint", "float", "double",
  };

  static const std::string type_min[8] = {
      "-128",
      "0",
      "-32,767",
      "0",
      "-2,147,483,647",
      "0",
      std::format("~{}", std::numeric_limits<float>::lowest()),
      std::format("~{}", std::numeric_limits<double>::lowest())};

  static const std::string type_max[8] = {
      "127",
      "255",
      "32,767",
      "65,535",
      "2,147,483,647",
      "4,294,967,295",
      std::format("~{}", std::numeric_limits<float>::max()),
      std::format("~{}", std::numeric_limits<double>::max())};

  uint8_t is_list_type = payload & 1u;
  uint8_t source = (payload >> 1u) & 0x7u;
  uint8_t dest = (payload >> 4u) & 0x7u;

  std::string result(prefix[is_list_type]);
  result += types[source];
  result += "' that ";
  result += type;
  result += " when converted to type '";
  result += types[dest];
  result += "' (value must be between ";
  result += type_min[dest];
  result += " and ";
  result += type_max[dest];
  result += ")";

  return result;
}

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
  std::error_condition default_error_condition(int value) const noexcept;
} kErrorCategory;

const char* ErrorCategory::name() const noexcept {
  return "plyodine::PlyReader";
}

std::string ErrorCategory::message(int condition) const {
  if (auto decoded = DecodeError(condition); decoded.has_value()) {
    switch (decoded->first) {
      case ErrorType::BAD_STREAM:
        return "The stream was not in 'good' state";
      case ErrorType::INVALID_CONVERSION:
        return InvalidConversionMessage(std::get<1>(*decoded));
      case ErrorType::UNEXPECTED_EOF:
        return MissingUnexpectedEofMessage(
            "The input ended earlier than expected (reached EOF but expected "
            "to find ",
            std::get<1>(*decoded));
      case ErrorType::UNEXPECTED_EOF_NO_PROPERTIES:
        return "The input ended earlier than expected (reached EOF but "
               "expected to find an element with no properties')";
      case ErrorType::MISMATCHED_LINE_ENDINGS:
        return "The input contained mismatched line endings";
      case ErrorType::INVALID_CHARACTERS:
        return "The input contained an invalid character in its data section "
               "(each line of input with format 'ascii' must contain only "
               "printable ASCII characters)";
      case ErrorType::MISSING_TOKEN:
        return MissingUnexpectedEofMessage(
            "The input contained a line in its data section with fewer tokens "
            "than expected (reached end of line but expected to find ",
            std::get<1>(*decoded));
      case ErrorType::UNUSED_TOKEN:
        return "The input contained a token in its data section that was not "
               "associated with any property";
      case ErrorType::FAILED_TO_PARSE:
        return FailedToParseMessage(std::get<1>(*decoded));
      case ErrorType::OUT_OF_RANGE:
        return OutOfRangeMessage(std::get<1>(*decoded));
      case ErrorType::OVERFLOW:
        return OverflowedUnderflowedMessage("overflowed",
                                            std::get<1>(*decoded));
      case ErrorType::UNDERFLOW:
        return OverflowedUnderflowedMessage("underflowed",
                                            std::get<1>(*decoded));
    }
  }

  return "Unknown Error";
}

std::error_condition ErrorCategory::default_error_condition(
    int value) const noexcept {
  return DecodeError(value)
             ? std::make_error_condition(std::errc::invalid_argument)
             : std::error_condition(value, *this);
}

std::error_code MakeBadStreamError() {
  int value = EncodeError(ErrorType::BAD_STREAM, 0u);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeInvalidConversionError(size_t source, size_t dest) {
  uint8_t payload = static_cast<uint8_t>(source | (dest << 4u));
  int value = EncodeError(ErrorType::INVALID_CONVERSION, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeUnexpectedEof(EntryType entry_type,
                                  PlyHeader::Property::Type data_type) {
  uint8_t payload =
      8u * static_cast<uint8_t>(entry_type) + static_cast<uint8_t>(data_type);
  int value = EncodeError(ErrorType::UNEXPECTED_EOF, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeUnexpectedEofNoProperties() {
  int value = EncodeError(ErrorType::UNEXPECTED_EOF_NO_PROPERTIES, 0);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeMismatchedLineEndings() {
  int value = EncodeError(ErrorType::MISMATCHED_LINE_ENDINGS, 0);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeInvalidCharacter() {
  int value = EncodeError(ErrorType::INVALID_CHARACTERS, 0);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeMissingToken(EntryType entry_type,
                                 PlyHeader::Property::Type data_type) {
  uint8_t payload =
      8u * static_cast<uint8_t>(entry_type) + static_cast<uint8_t>(data_type);
  int value = EncodeError(ErrorType::MISSING_TOKEN, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeUnusedToken() {
  int value = EncodeError(ErrorType::UNUSED_TOKEN, 0);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeFailedToParse(EntryType entry_type,
                                  PlyHeader::Property::Type data_type) {
  uint8_t payload =
      8u * static_cast<uint8_t>(entry_type) + static_cast<uint8_t>(data_type);
  int value = EncodeError(ErrorType::FAILED_TO_PARSE, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeOutOfRange(EntryType entry_type,
                               PlyHeader::Property::Type data_type) {
  uint8_t payload =
      8u * static_cast<uint8_t>(entry_type) + static_cast<uint8_t>(data_type);
  int value = EncodeError(ErrorType::OUT_OF_RANGE, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeUnderflowed(PlyHeader::Property::Type source,
                                PlyHeader::Property::Type dest, bool is_list) {
  uint8_t payload = static_cast<uint8_t>(is_list) |
                    (static_cast<uint8_t>(source) << 1u) |
                    (static_cast<uint8_t>(dest) << 4u);
  int value = EncodeError(ErrorType::UNDERFLOW, payload);
  return std::error_code(value, kErrorCategory);
}

std::error_code MakeOverflowed(PlyHeader::Property::Type source,
                               PlyHeader::Property::Type dest, bool is_list) {
  uint8_t payload = static_cast<uint8_t>(is_list) |
                    (static_cast<uint8_t>(source) << 1u) |
                    (static_cast<uint8_t>(dest) << 4u);
  int value = EncodeError(ErrorType::OVERFLOW, payload);
  return std::error_code(value, kErrorCategory);
}

template <typename T>
PlyHeader::Property::Type GetDataType() {
  if constexpr (std::is_same_v<T, int8_t>) {
    return PlyHeader::Property::Type::CHAR;
  } else if constexpr (std::is_same_v<T, uint8_t>) {
    return PlyHeader::Property::Type::UCHAR;
  } else if constexpr (std::is_same_v<T, int16_t>) {
    return PlyHeader::Property::Type::SHORT;
  } else if constexpr (std::is_same_v<T, uint16_t>) {
    return PlyHeader::Property::Type::USHORT;
  } else if constexpr (std::is_same_v<T, int32_t>) {
    return PlyHeader::Property::Type::INT;
  } else if constexpr (std::is_same_v<T, uint32_t>) {
    return PlyHeader::Property::Type::UINT;
  } else if constexpr (std::is_same_v<T, float>) {
    return PlyHeader::Property::Type::FLOAT;
  } else {
    static_assert(std::is_same_v<T, double>);
    return PlyHeader::Property::Type::DOUBLE;
  }
}

using ContextData =
    std::tuple<int8_t, std::vector<int8_t>, uint8_t, std::vector<uint8_t>,
               int16_t, std::vector<int16_t>, uint16_t, std::vector<uint16_t>,
               int32_t, std::vector<int32_t>, uint32_t, std::vector<uint32_t>,
               float, std::vector<float>, double, std::vector<double>>;

struct Context final {
  ContextData data;
  std::string_view line_ending;
  std::string storage;
  std::string_view line;
  std::string_view token;
  bool eof = false;
};

using AppendFunc = void (*)(Context&);
using ConvertFunc = std::error_code (*)(Context&, EntryType);
using Handler = std::function<std::error_code(Context&)>;
using OnConversionErrorFunc = std::function<std::error_code(
    const std::string&, const std::string&, std::error_code)>;
using ReadFunc = std::error_code (*)(std::istream&, Context&, EntryType);

std::error_code ReadNextLine(std::istream& stream, Context& context,
                             std::error_code end_of_file_error) {
  std::string_view line_ending = context.line_ending;

  context.storage.clear();
  context.eof = true;

  char c;
  while (stream.get(c)) {
    if (c == line_ending[0]) {
      line_ending.remove_prefix(1);

      while (!line_ending.empty()) {
        if (!stream.get(c) || c != line_ending[0]) {
          return MakeMismatchedLineEndings();
        }

        line_ending.remove_prefix(1);
      }

      context.eof = false;
      break;
    }

    if (c == '\r' || c == '\n') {
      return MakeMismatchedLineEndings();
    }

    if (c == '\t') {
      c = ' ';
    }

    if (!std::isprint(c)) {
      return MakeInvalidCharacter();
    }

    if (c == ' ') {
      if (context.storage.empty() || context.storage.back() == ' ') {
        continue;
      }
    }

    context.storage.push_back(c);
  }

  if (stream.fail() && !stream.eof()) {
    return std::io_errc::stream;
  }

  if (context.storage.empty() && context.eof) {
    return end_of_file_error;
  }

  context.line = context.storage;

  return std::error_code();
}

std::error_code ReadNextToken(Context& context, bool is_float,
                              std::error_code missing_token_error,
                              std::error_code end_of_line_error) {
  size_t prefix_length = context.line.find_first_not_of(' ');
  if (prefix_length == std::string_view::npos) {
    return context.eof ? end_of_line_error : missing_token_error;
  }

  context.line.remove_prefix(prefix_length);

  size_t token_length = context.line.find(' ');
  if (token_length == std::string::npos) {
    token_length = context.line.length();
  }

  context.token = context.line.substr(0u, token_length);
  context.line.remove_prefix(token_length);

  return std::error_code();
}

template <typename T>
std::error_code ReadASCII(std::istream& input, Context& context,
                          EntryType entry_type) {
  if (std::error_code error =
          ReadNextToken(context, std::is_floating_point_v<T>,
                        MakeMissingToken(entry_type, GetDataType<T>()),
                        MakeUnexpectedEof(entry_type, GetDataType<T>()));
      error) {
    return error;
  }

  const char* start = context.token.data();
  const char* end = start + context.token.size();

  bool out_of_range = false;
  if constexpr (std::is_unsigned_v<T>) {
    out_of_range = start[0] == '-';
    if (out_of_range) {
      start += 1;
    }
  }

  T value{};
  std::from_chars_result result = std::from_chars(start, end, value);
  if (result.ec == std::errc::invalid_argument || result.ptr != end) {
    return MakeFailedToParse(entry_type, GetDataType<T>());
  } else if (result.ec == std::errc::result_out_of_range || out_of_range) {
    return MakeOutOfRange(entry_type, GetDataType<T>());
  }

  if (entry_type == EntryType::LIST_SIZE && value < 0) {
    return MakeOutOfRange(EntryType::LIST_SIZE, GetDataType<T>());
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code ReadBinary(std::istream& stream, Context& context,
                           EntryType entry_type) {
  T value{};
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
  if (stream.fail()) {
    if (stream.eof()) {
      return MakeUnexpectedEof(entry_type, GetDataType<T>());
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  if (entry_type == EntryType::LIST_SIZE && value < 0) {
    return MakeOutOfRange(EntryType::LIST_SIZE, GetDataType<T>());
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::floating_point T>
std::error_code ReadBinary(std::istream& stream, Context& context,
                           EntryType entry_type) {
  std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t> value{};

  stream.read(reinterpret_cast<char*>(&value), sizeof(value));
  if (stream.fail()) {
    if (stream.eof()) {
      return MakeUnexpectedEof(entry_type, GetDataType<T>());
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  if (entry_type == EntryType::LIST_SIZE && value < 0) {
    return MakeOutOfRange(EntryType::LIST_SIZE, GetDataType<T>());
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
std::error_code Convert(Context& context, EntryType entry_type) {
  static_assert(std::is_floating_point_v<Source> ==
                std::is_floating_point_v<Dest>);

  if constexpr (!std::is_same_v<Source, Dest>) {
    if constexpr (std::is_floating_point_v<Source>) {
      if constexpr (sizeof(Dest) < sizeof(Source)) {
        if (std::isfinite(std::get<Source>(context.data))) {
          if (std::get<Source>(context.data) <
              std::numeric_limits<Dest>::lowest()) {
            return MakeUnderflowed(GetDataType<Source>(), GetDataType<Dest>(),
                                   entry_type == EntryType::LIST_VALUE);
          }

          if (std::get<Source>(context.data) >
              std::numeric_limits<Dest>::max()) {
            return MakeOverflowed(GetDataType<Source>(), GetDataType<Dest>(),
                                  entry_type == EntryType::LIST_VALUE);
          }
        }
      }
    } else {
      if constexpr (std::is_signed_v<Source> && !std::is_signed_v<Dest>) {
        if (std::get<Source>(context.data) < 0) {
          return MakeUnderflowed(GetDataType<Source>(), GetDataType<Dest>(),
                                 entry_type == EntryType::LIST_VALUE);
        }
      } else if constexpr (std::is_signed_v<Source> && std::is_signed_v<Dest> &&
                           sizeof(Dest) < sizeof(Source)) {
        if (std::get<Source>(context.data) < std::numeric_limits<Dest>::min()) {
          return MakeUnderflowed(GetDataType<Source>(), GetDataType<Dest>(),
                                 entry_type == EntryType::LIST_VALUE);
        }
      }

      if constexpr (sizeof(Source) > sizeof(Dest) ||
                    (sizeof(Source) == sizeof(Dest) &&
                     !std::is_signed_v<Source> && std::is_signed_v<Dest>)) {
        if (std::get<Source>(context.data) >
            static_cast<Source>(std::numeric_limits<Dest>::max())) {
          return MakeOverflowed(GetDataType<Source>(), GetDataType<Dest>(),
                                entry_type == EntryType::LIST_VALUE);
        }
      }
    }

    std::get<Dest>(context.data) =
        static_cast<Dest>(std::get<Source>(context.data));
  }

  return std::error_code();
}

template <PlyHeader::Property::Type Source, PlyHeader::Property::Type Dest>
consteval ConvertFunc GetConvertFunc() {
  using SourceType =
      std::tuple_element_t<static_cast<size_t>(Source) * 2, ContextData>;
  using DestType =
      std::tuple_element_t<static_cast<size_t>(Dest) * 2, ContextData>;

  if constexpr (std::is_floating_point_v<SourceType> ==
                std::is_floating_point_v<DestType>) {
    return Convert<SourceType, DestType>;
  }

  return nullptr;
}

template <PlyHeader::Property::Type Source>
consteval std::array<ConvertFunc, 8> GetConvertFuncs() {
  return {
      GetConvertFunc<Source, PlyHeader::Property::Type::CHAR>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::UCHAR>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::SHORT>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::USHORT>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::INT>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::UINT>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::FLOAT>(),
      GetConvertFunc<Source, PlyHeader::Property::Type::DOUBLE>(),
  };
}

ConvertFunc GetConvertFunc(PlyHeader::Property::Type source,
                           PlyHeader::Property::Type dest) {
  static constexpr std::array<ConvertFunc, 8> conversion_funcs[8] = {
      GetConvertFuncs<PlyHeader::Property::Type::CHAR>(),
      GetConvertFuncs<PlyHeader::Property::Type::UCHAR>(),
      GetConvertFuncs<PlyHeader::Property::Type::SHORT>(),
      GetConvertFuncs<PlyHeader::Property::Type::USHORT>(),
      GetConvertFuncs<PlyHeader::Property::Type::INT>(),
      GetConvertFuncs<PlyHeader::Property::Type::UINT>(),
      GetConvertFuncs<PlyHeader::Property::Type::FLOAT>(),
      GetConvertFuncs<PlyHeader::Property::Type::DOUBLE>(),
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

template <typename PropertyCallback>
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
    if (std::error_code error =
            read_length_(stream, context, EntryType::LIST_SIZE);
        error) {
      return error;
    }

    convert_length_(context, EntryType::LIST_SIZE);

    length = std::get<uint32_t>(context.data);
  }

  for (uint32_t i = 0; i < length; i++) {
    EntryType entry_type =
        read_length_ ? EntryType::LIST_VALUE : EntryType::VALUE;
    if (std::error_code error = read_(stream, context, entry_type); error) {
      return error;
    }

    if (std::error_code error = convert_(context, entry_type); error) {
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

template <typename PropertyCallback>
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
    return MakeBadStreamError();
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

    std::map<std::string, PropertyCallback>& property_callbacks =
        actual_callbacks[element.name];
    for (const auto& property : element.properties) {
      property_callbacks[property.name] = MakeEmptyCallback<PropertyCallback>(
          property.data_type, property.list_type.has_value());
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
      continue;
    }

    for (auto& [property_name, property_callback] : element_callbacks) {
      auto property_iter = element_iter->second.find(property_name);
      if (property_iter == element_iter->second.end()) {
        continue;
      }

      if (IsInvalidConversion(property_iter->second.index(),
                              property_callback.index())) {
        return MakeInvalidConversionError(property_iter->second.index(),
                                          property_callback.index());
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
                    std::error_code error) -> std::error_code {
            auto [type, payload] = *DecodeError(error.value());
            auto [source, dest, is_list] = *DecodeOverUnderFlowPayload(payload);

            ConversionFailureReason reason =
                ConversionFailureReason::UNSIGNED_INTEGER_UNDERFLOW;
            if (type == ErrorType::UNDERFLOW) {
              if (dest == PlyHeader::Property::Type::FLOAT) {
                reason = ConversionFailureReason::FLOAT_UNDERFLOW;
              } else if (dest == PlyHeader::Property::Type::CHAR ||
                         dest == PlyHeader::Property::Type::SHORT ||
                         dest == PlyHeader::Property::Type::INT) {
                reason = ConversionFailureReason::SIGNED_INTEGER_UNDERFLOW;
              }
            } else {
              if (dest == PlyHeader::Property::Type::FLOAT) {
                reason = ConversionFailureReason::FLOAT_OVERFLOW;
              } else {
                reason = ConversionFailureReason::INTEGER_OVERFLOW;
              }
            }

            if (std::error_code error =
                    OnConversionFailure(element_name, property_name, reason);
                error) {
              return error;
            }

            return error;
          },
          element.name, property.name);
    }
  }

  Context context;
  context.line_ending = header->line_ending;
  for (size_t element_index = 0; element_index < header->elements.size();
       element_index++) {
    const PlyHeader::Element& element = header->elements[element_index];
    for (size_t instance = 0; instance < element.num_in_file; instance++) {
      if (header->format == PlyHeader::Format::ASCII) {
        std::error_code eof_error = MakeUnexpectedEofNoProperties();
        if (!element.properties.empty()) {
          const PlyHeader::Property& property = element.properties.front();
          if (property.list_type) {
            eof_error =
                MakeUnexpectedEof(EntryType::LIST_SIZE, *property.list_type);
          } else {
            eof_error =
                MakeUnexpectedEof(EntryType::VALUE, *property.list_type);
          }
        }

        if (std::error_code error = ReadNextLine(stream, context, eof_error);
            error) {
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
        std::error_code error =
            ReadNextToken(context, false, MakeUnusedToken(), MakeUnusedToken());
        if (!error) {
          return MakeUnusedToken();
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
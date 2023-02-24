#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace plyodine {

class Error final {
 public:
  enum ErrorCode { IO_ERROR, PARSING_ERROR };

  Error(ErrorCode code, std::string_view message) noexcept
      : code_(code), message_(message) {}

  ErrorCode Code() const noexcept { return code_; }
  std::string_view Message() const noexcept { return message_; }

  static Error IoError(std::string_view message) noexcept {
    return Error(IO_ERROR, message);
  }

  static Error ParsingError(std::string_view message) noexcept {
    return Error(PARSING_ERROR, message);
  }

 private:
  ErrorCode code_;
  std::string_view message_;
};

namespace internal {

enum class Format { ASCII, BINARY_LITTLE_ENDIAN, BINARY_BIG_ENDIAN };

enum class Type { INT8, UINT8, INT16, UINT16, INT32, UINT32, FLOAT, DOUBLE };

struct Property {
  std::string name;
  Type data_type;
  std::optional<Type> list_type;
};

struct Element {
  std::string name;
  size_t num_in_file;
  std::vector<Property> properties;
};

struct Header {
  Format format;
  uint8_t major_version;
  uint8_t minor_version;
  std::vector<std::string> comments;
  std::vector<Element> elements;
};

std::expected<Header, Error> ParseHeader(std::istream& input);

}  // namespace internal

class PlyReader {
 public:
  void ParseFrom(std::istream& input);

 private:
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
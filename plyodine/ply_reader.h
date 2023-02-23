#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

namespace plyodine {

class Error {
public:
 enum class Type {
  EARLY_EOF
 };

 Error(Type type, std::string_view message)
 : type_(type), message_(message) { }

 Type Type() const { return type_; }
 std::string_view Message() const { return message_; }

 private:
 const Type type_;
 const std::string_view message_;
};

namespace internal {

enum class Format {
    ASCII,
    BINARY
};

enum class Type {
  INT8, UINT8, INT16, UINT16, INT32, UINT32, FLOAT32, FLOAT64
};

struct Property {
 std::string name;
 Type type;
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

std::expected<PlyHeader, Error> ParseHeader(std::istream& input);

}  // namespace internal

class PlyReader {
public:
 void ParseFrom(std::istream& input);
private:
 
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
#ifndef _PLYODINE_PLY_HEADER_
#define _PLYODINE_PLY_HEADER_

#include <cstdint>
#include <expected>
#include <istream>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace plyodine {

// A struct describing the contents of a PLY header.
struct PlyHeader final {
  // A struct describing a property.
  struct Property final {
    // The set of supported types.
    enum class Type {
      CHAR,    // Equivalent to int8_t
      UCHAR,   // Equivalent to uint8_t
      SHORT,   // Equivalent to int16_t
      USHORT,  // Equivalent to uint16_t
      INT,     // Equivalent to int32_t
      UINT,    // Equivalent to uint32_t
      FLOAT,   // Equivalent to float
      DOUBLE   // Equivalent to double
    };

    // The name of the property.
    std::string name;

    // The type of the property's values.
    Type data_type;

    // If the property is a list, this is set to the type used to store the
    // list's size and will never be `FLOAT` or `DOUBLE`. If the property is not
    // a list this will be set to `std::nullopt`.
    std::optional<Type> list_type;
  };

  // A struct describing an element.
  struct Element final {
    // The name of the element.
    std::string name;

    // The number of instances of the element.
    uintmax_t num_in_file;

    // An ordered list of the properties of the element.
    std::vector<Property> properties;
  };

  // The format of the data following the header.
  enum class Format { ASCII, BINARY_BIG_ENDIAN, BINARY_LITTLE_ENDIAN } format;

  // A string containing the line ending used in the header.
  std::string line_ending;

  // The major PLY version number in the header.
  uint8_t major_version;

  // The minor PLY version number in the header.
  uint8_t minor_version;

  // A list of comments in the header that use the `comment` keyword.
  std::vector<std::string> comments;

  // A list of comments in the header that use the `obj_info` keyword.
  std::vector<std::string> object_info;

  // An ordered list of the elements described in the header.
  std::vector<Element> elements;
};

// Reads the PLY header from the input stream. On success, the function returns
// a struct describing the contents of the PLY header and stream will have been
// advanced past the header to the start of the data section. On failure,
// returns an `std::error_code` containing a non-zero value and the stream will
// be left in an undefined state.
//
// NOTE: Behavior is undefined if input is not a binary stream
std::expected<PlyHeader, std::error_code> ReadPlyHeader(std::istream& input);

}  // namespace plyodine

#endif  // _PLYODINE_PLY_HEADER_

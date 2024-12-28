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

struct PlyHeader final {
  struct Property final {
    enum Type { INT8, UINT8, INT16, UINT16, INT32, UINT32, FLOAT, DOUBLE };
    std::string name;
    Type data_type;
    std::optional<Type> list_type;
  };

  struct Element final {
    std::string name;
    uintmax_t num_in_file;
    std::vector<Property> properties;
  };

  enum Format { ASCII, BINARY_LITTLE_ENDIAN, BINARY_BIG_ENDIAN } format;
  std::string line_ending;
  uint8_t major_version;
  uint8_t minor_version;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  std::vector<Element> elements;
};

// NOTE: Behavior is undefined if input is not a binary stream
std::expected<PlyHeader, std::error_code> ReadPlyHeader(std::istream& input);

}  // namespace plyodine

#endif  // _PLYODINE_PLY_HEADER_

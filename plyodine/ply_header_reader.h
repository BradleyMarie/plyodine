#ifndef _PLYODINE_PLY_HEADER_READER_
#define _PLYODINE_PLY_HEADER_READER_

#include <cstdint>
#include <expected>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
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
    uint64_t num_in_file;
    std::vector<Property> properties;
  };

  enum Format { ASCII, BINARY_LITTLE_ENDIAN, BINARY_BIG_ENDIAN } format;
  std::string_view line_ending;
  uint8_t major_version;
  uint8_t minor_version;
  std::vector<std::string> comments;
  std::vector<std::string> object_info;
  std::vector<Element> elements;
};

std::expected<PlyHeader, std::string_view> ReadPlyHeader(std::istream& input);

}  // namespace plyodine

#endif  // _PLYODINE_PLY_HEADER_READER_

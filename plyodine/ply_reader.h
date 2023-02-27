#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <expected>
#include <istream>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
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

struct Header final {
  struct Element final {
    struct Property final {
      std::string name;
      Type data_type;
      std::optional<Type> list_type;
    };

    std::string name;
    size_t num_in_file;
    std::vector<Property> properties;
  };

  Format format;
  std::string_view line_ending;
  uint8_t major_version;
  uint8_t minor_version;
  std::vector<std::string> comments;
  std::vector<Element> elements;
};

std::expected<Header, Error> ParseHeader(std::istream& input);

struct Element final {
  template <typename T>
  struct List final {
    struct Entry final {
      size_t index;
      size_t size;
    };

    std::vector<Entry> entries;
    std::vector<T> data;
  };

  typedef std::variant<std::vector<int8_t>, std::vector<uint8_t>,
                       std::vector<int16_t>, std::vector<uint16_t>,
                       std::vector<int32_t>, std::vector<uint32_t>,
                       std::vector<float>, std::vector<double>, List<int8_t>,
                       List<uint8_t>, List<int16_t>, List<uint16_t>,
                       List<int32_t>, List<uint32_t>, List<float>, List<double>>
      Property;

  std::vector<Property> properties;
};

std::expected<std::vector<Element>, Error> ReadData(std::istream& input,
                                                    const Header& header);

}  // namespace internal

enum class PropertyType {
  INT8,
  INT8_LIST,
  UINT8,
  UINT8_LIST,
  INT16,
  INT16_LIST,
  UINT16,
  UINT16_LIST,
  INT32,
  INT32_LIST,
  UINT32,
  UINT32_LIST,
  FLOAT,
  FLOAT_LIST,
  DOUBLE,
  DOUBLE_LIST
};

class PlyReader final {
 public:
  std::optional<Error> ReadFrom(std::istream& input);

  // Comments
  std::span<const std::string> GetComments() const;

  // Elements
  std::span<const std::string> GetElements() const;

  // Properties
  std::optional<std::span<const std::string>> GetProperties(
      std::string_view element_name) const;

  // Property Type
  std::optional<PropertyType> GetPropertyType(
      std::string_view element_name, std::string_view property_name) const;

  // Structured Property Access
  std::optional<std::span<const int8_t>> GetPropertyInt8(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const int8_t>>> GetPropertyListInt8(
      std::string_view element_name, std::string_view property_name) const;

  std::optional<std::span<const uint8_t>> GetPropertyUInt8(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const uint8_t>>> GetPropertyListUInt8(
      std::string_view element_name, std::string_view property_name) const;

  std::optional<std::span<const int16_t>> GetPropertyInt16(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const int16_t>>> GetPropertyListInt16(
      std::string_view element_name, std::string_view property_name) const;

  std::optional<std::span<const uint16_t>> GetPropertyUInt16(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const uint16_t>>>
  GetPropertyListUInt16(std::string_view element_name,
                        std::string_view property_name) const;

  std::optional<std::span<const int32_t>> GetPropertyInt32(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const int32_t>>> GetPropertyListInt32(
      std::string_view element_name, std::string_view property_name) const;

  std::optional<std::span<const uint32_t>> GetPropertyUInt32(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const uint32_t>>>
  GetPropertyListUInt32(std::string_view element_name,
                        std::string_view property_name) const;

  std::optional<std::span<const float>> GetPropertyFloat(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const float>>> GetPropertyListFloat(
      std::string_view element_name, std::string_view property_name) const;

  std::optional<std::span<const double>> GetPropertyDouble(
      std::string_view element_name, std::string_view property_name) const;
  std::optional<std::span<const std::span<const double>>> GetPropertyListDouble(
      std::string_view element_name, std::string_view property_name) const;

  // Unstructured Property Access
  typedef std::variant<
      std::span<const int8_t>, std::span<const std::span<const int8_t>>,
      std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
      std::span<const int16_t>, std::span<const std::span<const int16_t>>,
      std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
      std::span<const int32_t>, std::span<const std::span<const int32_t>>,
      std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
      std::span<const float>, std::span<const std::span<const float>>,
      std::span<const double>, std::span<const std::span<const double>>>
      Property;

  std::optional<Property> GetProperty(std::string_view element_name,
                                      std::string_view property_name) const;

 private:
  std::vector<internal::Element> elements_;
  std::vector<std::string> comments_;
  std::vector<std::string> element_names_;
  std::map<std::string, std::vector<std::string>> property_names_;
  std::map<std::string, std::map<std::string, const Property*>> properties_;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
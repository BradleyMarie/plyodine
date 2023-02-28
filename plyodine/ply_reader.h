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

#include "plyodine/ply_property.h"

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
  struct Single final {
    std::vector<T> entries;
  };

  template <typename T>
  struct List final {
    struct Extents final {
      size_t index;
      size_t size;
    };

    std::vector<Extents> extents;
    std::vector<std::span<const T>> entries;
    std::vector<T> data;
  };

  typedef std::variant<Single<int8_t>, Single<uint8_t>, Single<int16_t>,
                       Single<uint16_t>, Single<int32_t>, Single<uint32_t>,
                       Single<float>, Single<double>, List<int8_t>,
                       List<uint8_t>, List<int16_t>, List<uint16_t>,
                       List<int32_t>, List<uint32_t>, List<float>, List<double>>
      Property;

  std::vector<Property> properties;
};

std::expected<std::vector<Element>, Error> ReadData(std::istream& input,
                                                    const Header& header);

}  // namespace internal

class PlyReader final {
 public:
  std::optional<Error> ReadFrom(std::istream& input);

  std::span<const std::string> GetComments() const;

  std::span<const std::string> GetElements() const;

  std::optional<std::span<const std::string>> GetProperties(
      std::string_view element_name) const;

  std::optional<Property::Type> GetPropertyType(
      std::string_view element_name, std::string_view property_name) const;

  const Property* GetProperty(std::string_view element_name,
                              std::string_view property_name) const;

 private:
  std::vector<internal::Element> elements_;
  std::vector<std::string> comments_;
  std::vector<std::string> element_names_;
  std::map<std::string, std::vector<std::string>, std::less<>> property_names_;
  std::map<std::string, std::map<std::string, Property, std::less<>>,
           std::less<>>
      properties_;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
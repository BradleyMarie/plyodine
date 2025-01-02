#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <cstdint>
#include <generator>
#include <map>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace plyodine {

// The base class enabling PLY serialization.
class PlyWriter {
 public:
  // Writes a PLY file to the output stream in the binary format matching the
  // system's native endianness.
  //
  // NOTE: Behavior is undefined if stream is not a binary stream.
  std::error_code WriteTo(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the ASCII format. Most clients
  // should prefer WriteTo over this.
  //
  // NOTE: Behavior is undefined if stream is not a binary stream.
  std::error_code WriteToASCII(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the binary big-endian format.
  // Most clients should prefer WriteTo over this.
  //
  // NOTE: Behavior is undefined if stream is not a binary stream.
  std::error_code WriteToBigEndian(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the binary little-endian format.
  // Most clients should prefer WriteTo over these.
  //
  // NOTE: Behavior is undefined if stream is not a binary stream.
  std::error_code WriteToLittleEndian(std::ostream& stream) const;

 protected:
  // A generator that yields the values of a char property.
  using CharPropertyGenerator = std::generator<int8_t>;

  // A generator that yields the values of a char property list.
  using CharPropertyListGenerator = std::generator<std::span<const int8_t>>;

  // A generator that yields the values of a uchar property.
  using UCharPropertyGenerator = std::generator<uint8_t>;

  // A generator that yields the values of a uchar property list.
  using UCharPropertyListGenerator = std::generator<std::span<const uint8_t>>;

  // A generator that yields the values of a short property.
  using ShortPropertyGenerator = std::generator<int16_t>;

  // A generator that yields the values of a short property list.
  using ShortPropertyListGenerator = std::generator<std::span<const int16_t>>;

  // A generator that yields the values of a ushort property.
  using UShortPropertyGenerator = std::generator<uint16_t>;

  // A generator that yields the values of a ushort property list.
  using UShortPropertyListGenerator = std::generator<std::span<const uint16_t>>;

  // A generator that yields the values of an int property.
  using IntPropertyGenerator = std::generator<int32_t>;

  // A generator that yields the values of an int property list.
  using IntPropertyListGenerator = std::generator<std::span<const int32_t>>;

  // A generator that yields the values of a uint property.
  using UIntPropertyGenerator = std::generator<uint32_t>;

  // A generator that yields the values of a uint property list.
  using UIntPropertyListGenerator = std::generator<std::span<const uint32_t>>;

  // A generator that yields the values of a float property.
  using FloatPropertyGenerator = std::generator<float>;

  // A generator that yields the values of a float property list.
  using FloatPropertyListGenerator = std::generator<std::span<const float>>;

  // A generator that yields the values of a double property.
  using DoublePropertyGenerator = std::generator<double>;

  // A generator that yields the values of a double property list.
  using DoublePropertyListGenerator = std::generator<std::span<const double>>;

  // A variant that contains the generator for a property. The type of the
  // variant determines the type of the property in the output.
  using PropertyGenerator =
      std::variant<CharPropertyGenerator, CharPropertyListGenerator,
                   UCharPropertyGenerator, UCharPropertyListGenerator,
                   ShortPropertyGenerator, ShortPropertyListGenerator,
                   UShortPropertyGenerator, UShortPropertyListGenerator,
                   IntPropertyGenerator, IntPropertyListGenerator,
                   UIntPropertyGenerator, UIntPropertyListGenerator,
                   FloatPropertyGenerator, FloatPropertyListGenerator,
                   DoublePropertyGenerator, DoublePropertyListGenerator>;

  // This function is implemented by derived classes and provides the details
  // needed by PlyWriter in order to fill out the header of the file as well as
  // generators for all of the properties in the file.
  //
  // `num_num_element_instances`: The number of instances of each element type.
  //
  // `property_generators`: The generators for each property in the file, keyed
  // from element name to property name to generator. The type of the property
  // is inferred from the type of the generator.
  //
  // `comments`: Comments to include in the header using the comment prefix.
  //
  // `object_info`: Comments to include in the header using the obj_info prefix.
  //
  // The value of the std::error_code returned must be zero on success.
  virtual std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          property_generators,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const = 0;

  // The type used for the size of a property list.
  enum class ListSizeType : unsigned int {
    UINT8 = 0,
    UINT16 = 1,
    UINT32 = 2,
  };

  // This function is implemented by derived classes and is called by PlyWriter
  // to determine the width of the size that should be used for each propery
  // lists in the output.
  virtual PlyWriter::ListSizeType GetPropertyListSizeType(
      const std::string& element_name,
      const std::string& property_name) const = 0;
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
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

// The base class enabling PLY serlialization.
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
  typedef std::generator<int8_t> Int8PropertyGenerator;

  // A generator that yields the values of a char property list.
  typedef std::generator<std::span<const int8_t>> Int8PropertyListGenerator;

  // A generator that yields the values of a uchar property.
  typedef std::generator<uint8_t> UInt8PropertyGenerator;

  // A generator that yields the values of a uchar property list.
  typedef std::generator<std::span<const uint8_t>> UInt8PropertyListGenerator;

  // A generator that yields the values of a short property.
  typedef std::generator<int16_t> Int16PropertyGenerator;

  // A generator that yields the values of a short property list.
  typedef std::generator<std::span<const int16_t>> Int16PropertyListGenerator;

  // A generator that yields the values of a ushort property.
  typedef std::generator<uint16_t> UInt16PropertyGenerator;

  // A generator that yields the values of a ushort property list.
  typedef std::generator<std::span<const uint16_t>> UInt16PropertyListGenerator;

  // A generator that yields the values of an int property.
  typedef std::generator<int32_t> Int32PropertyGenerator;

  // A generator that yields the values of an int property list.
  typedef std::generator<std::span<const int32_t>> Int32PropertyListGenerator;

  // A generator that yields the values of a uint property.
  typedef std::generator<uint32_t> UInt32PropertyGenerator;

  // A generator that yields the values of a uint property list.
  typedef std::generator<std::span<const uint32_t>> UInt32PropertyListGenerator;

  // A generator that yields the values of a float property.
  typedef std::generator<float> FloatPropertyGenerator;

  // A generator that yields the values of a float property list.
  typedef std::generator<std::span<const float>> FloatPropertyListGenerator;

  // A generator that yields the values of a double property.
  typedef std::generator<double> DoublePropertyGenerator;

  // A generator that yields the values of a double property list.
  typedef std::generator<std::span<const double>> DoublePropertyListGenerator;

  // A variant that contains the generator for a property. The type of the
  // generator determines the type of the property in the output.
  typedef std::variant<Int8PropertyGenerator, Int8PropertyListGenerator,
                       UInt8PropertyGenerator, UInt8PropertyListGenerator,
                       Int16PropertyGenerator, Int16PropertyListGenerator,
                       UInt16PropertyGenerator, UInt16PropertyListGenerator,
                       Int32PropertyGenerator, Int32PropertyListGenerator,
                       UInt32PropertyGenerator, UInt32PropertyListGenerator,
                       FloatPropertyGenerator, FloatPropertyListGenerator,
                       DoublePropertyGenerator, DoublePropertyListGenerator>
      ValueGenerator;

  // This function is implemented by derived classes and provides the details
  // needed by PlyWriter in order to fill out the header of the file as well as
  // generators for all of the properties in the file.
  //
  // `num_num_element_instances`: The number of instances of each element type.
  //
  // `generators`: The generators for each property in the file, keyed from
  // element name to property name to generator. The type of the property is
  // inferred from the type of the generator.
  //
  // `comments`: Comments to include in the header using the comment prefix.
  //
  // `object_info`: Comments to include in the header using the obj_info prefix.
  virtual std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, ValueGenerator>>& generators,
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
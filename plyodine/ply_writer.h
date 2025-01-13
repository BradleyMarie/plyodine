#ifndef _PLYODINE_PLY_WRITER_
#define _PLYODINE_PLY_WRITER_

#include <cstdint>
#include <generator>
#include <map>
#include <memory>
#include <ostream>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace plyodine {

// The base class enabling PLY serialization.
//
// Derived classes should implement either `DelegateTo` or `Start` (or both).
class PlyWriter {
 public:
  virtual ~PlyWriter() = default;

  // Writes a PLY file to the output stream in the binary format matching the
  // system's native endianness.
  //
  // On success returns an `std::error_code` with a zero value. On failure,
  // returns an `std::error_code` with a non-zero value and the stream will be
  // left in an undetermined state.
  //
  // NOTE: Behavior is undefined if `stream` is not a binary stream.
  std::error_code WriteTo(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the ASCII format.
  //
  // On success returns an `std::error_code` with a zero value. On failure,
  // returns an `std::error_code` with a non-zero value and the stream will be
  // left in an undetermined state.
  //
  // Most clients should prefer WriteTo over this.
  //
  // NOTE: Behavior is undefined if `stream` is not a binary stream.
  std::error_code WriteToASCII(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the binary big-endian format.
  //
  // On success returns an `std::error_code` with a zero value. On failure,
  // returns an `std::error_code` with a non-zero value and the stream will be
  // left in an undetermined state.
  //
  // Most clients should prefer WriteTo over this.
  //
  // NOTE: Behavior is undefined if `stream` is not a binary stream.
  std::error_code WriteToBigEndian(std::ostream& stream) const;

  // Writes a PLY file to the output stream in the binary little-endian format.
  //
  // On success returns an `std::error_code` with a zero value. On failure,
  // returns an `std::error_code` with a non-zero value and the stream will be
  // left in an undetermined state.
  //
  // Most clients should prefer WriteTo over these.
  //
  // NOTE: Behavior is undefined if `stream` is not a binary stream.
  std::error_code WriteToLittleEndian(std::ostream& stream) const;

 protected:
  // The constructor of PlyWriter is protected in order to reduce the likelihood
  // of accidentally instantiating this class directly.
  PlyWriter() = default;

  // The type used for the size of a property list.
  enum class ListSizeType {
    UCHAR = 0,   // Equivalent to uint8_t
    USHORT = 1,  // Equivalent to uint16_t
    UINT = 2,    // Equivalent to uint32_t
  };

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

 private:
  // This function may be implemented by derived classes in order to delegate
  // PLY writing to a different writer. This function is called before `Start`
  // and may be called recursively on the returned PlyWriter until reaching a
  // PlyWriter that does not delegate. Once this writer has been reached, all
  // callbacks will go to that object instead of this one.
  //
  // The delegate will be destroyed after writing has finished.
  virtual std::unique_ptr<const PlyWriter> DelegateTo() const {
    return nullptr;
  }

  // This function is implemented by derived classes and provides the details
  // needed by PlyWriter in order to fill out the header of the file as well as
  // generators for all of the properties in the file.
  //
  // `num_num_element_instances`: The number of instances of each element.
  //
  // `property_generators`: The generators for each property in the file, keyed
  // from element name to property name to generator. The type of the property
  // is inferred from the type of the generator.
  //
  // `comments`: Comments to include in the header using the comment prefix.
  //
  // `object_info`: Comments to include in the header using the obj_info prefix.
  //
  // The value of the `std::error_code` returned must be zero on success.
  virtual std::error_code Start(
      std::map<std::string, uintmax_t>& num_element_instances,
      std::map<std::string, std::map<std::string, PropertyGenerator>>&
          property_generators,
      std::vector<std::string>& comments,
      std::vector<std::string>& object_info) const {
    return std::error_code();
  }

  // This function may be implemented by derived classes to control the width of
  // the size that will be used for each propery list in the output.
  virtual ListSizeType GetPropertyListSizeType(
      const std::string& element_name, const std::string& property_name) const {
    return ListSizeType::UINT;
  }
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_WRITER_
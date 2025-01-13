#ifndef _PLYODINE_PLY_READER_
#define _PLYODINE_PLY_READER_

#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <span>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace plyodine {

// The base class enabling PLY deserialization.
class PlyReader {
 public:
  virtual ~PlyReader() = default;

  // Reads the input stream as a PLY file. On success, the function returns an
  // `std::error_code` containing a zero value and the stream will have been
  // advanced to the end of the data section of the input. On failure, returns
  // an `std::error_code` containing a non-zero value and the stream will be
  // left in an undefined state.
  //
  // NOTE: Behavior is undefined if `stream` is not a binary stream.
  std::error_code ReadFrom(std::istream& stream);

 protected:
  // The reason a type conversion failed.
  enum class ConversionFailureReason {
    // Attempted to convert a negative signed integer to an unsigned integer.
    UNSIGNED_INTEGER_UNDERFLOW = 0,

    // Attempted to narrow a negative integer that was out of range for the
    // destination signed integer type.
    SIGNED_INTEGER_UNDERFLOW = 1,

    // Attempted to narrow a positive integer that was out of range for the
    // destination integer type.
    INTEGER_OVERFLOW = 2,

    // Attempted to narrow a negative double value that was out of range for
    // float.
    FLOAT_UNDERFLOW = 3,

    // Attempted to narrow a positive double value that was out of range for
    // float.
    FLOAT_OVERFLOW = 4,
  };

  // A callback that receives the values of a char property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using CharPropertyCallback = std::function<std::error_code(int8_t)>;

  // A callback that receives the values of a char property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using CharPropertyListCallback =
      std::function<std::error_code(std::span<const int8_t>)>;

  // A callback that receives the values of a uchar property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UCharPropertyCallback = std::function<std::error_code(uint8_t)>;

  // A callback that receives the values of a uchar property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UCharPropertyListCallback =
      std::function<std::error_code(std::span<const uint8_t>)>;

  // A callback that receives the values of a short property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using ShortPropertyCallback = std::function<std::error_code(int16_t)>;

  // A callback that receives the values of a short property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using ShortPropertyListCallback =
      std::function<std::error_code(std::span<const int16_t>)>;

  // A callback that receives the values of a ushort property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UShortPropertyCallback = std::function<std::error_code(uint16_t)>;

  // A callback that receives the values of a ushort property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UShortPropertyListCallback =
      std::function<std::error_code(std::span<const uint16_t>)>;

  // A callback that receives the values of an int property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using IntPropertyCallback = std::function<std::error_code(int32_t)>;

  // A callback that receives the values of an int property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using IntPropertyListCallback =
      std::function<std::error_code(std::span<const int32_t>)>;

  // A callback that receives the values of a uint property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UIntPropertyCallback = std::function<std::error_code(uint32_t)>;

  // A callback that receives the values of a uint property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using UIntPropertyListCallback =
      std::function<std::error_code(std::span<const uint32_t>)>;

  // A callback that receives the values of a float property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using FloatPropertyCallback = std::function<std::error_code(float)>;

  // A callback that receives the values of a float property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using FloatPropertyListCallback =
      std::function<std::error_code(std::span<const float>)>;

  // A callback that receives the values of a double property.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using DoublePropertyCallback = std::function<std::error_code(double)>;

  // A callback that receives the values of a double property list.
  //
  // On success, returns an `std::error_code` with a zero-value.
  using DoublePropertyListCallback =
      std::function<std::error_code(std::span<const double>)>;

  // A variant that contains the callback for a property. The type of the
  // variant determines the type of the property in the input.
  typedef std::variant<CharPropertyCallback, CharPropertyListCallback,
                       UCharPropertyCallback, UCharPropertyListCallback,
                       ShortPropertyCallback, ShortPropertyListCallback,
                       UShortPropertyCallback, UShortPropertyListCallback,
                       IntPropertyCallback, IntPropertyListCallback,
                       UIntPropertyCallback, UIntPropertyListCallback,
                       FloatPropertyCallback, FloatPropertyListCallback,
                       DoublePropertyCallback, DoublePropertyListCallback>
      PropertyCallback;

 private:
  // This function is implemented by derived classes to receive the details of
  // the PLY input from its header and to set up the callbacks to receive the
  // data values of the input.
  //
  // `num_num_element_instances`: The number of instances of each element.
  //
  // `callbacks`: The callbacks for each property in the file, keyed from
  // element name to property name to callback. The type of the property can be
  // inferred from the type of the callback. Each property in the input will be
  // present in `callbacks` when `Start` is invoked and will be populated with
  // an empty function. The type of the callback may also be changed in order
  // to receive the data in a different format then they are in the file.
  // Widening conversions (ex. char -> short) will always succeed. Narrowing
  // integer conversions will succeed as long as the narrowed value is still in
  // range for the narrowed type and conversions between signed and unsigned
  // integers will succeed as long as the value is non-negative. Narrowing
  // floating point conversions are lossy and will succeed as long as the value
  // remains finite after narrowing. Conversions between integer and floating
  // point types are not supported and will be rejected by PlyReader. In the
  // event a supported conversion fails, the failure can be observed by
  // implementing `OnConversionFailure`.
  //
  // `comments`: The comments in the that used the comment prefix.
  //
  // `object_info`: The comments in the header that used the obj_info prefix.
  //
  // The value of the `std::error_code` returned must be zero on success.
  virtual std::error_code Start(
      std::map<std::string, uintmax_t> num_element_instances,
      std::map<std::string, std::map<std::string, PropertyCallback>>& callbacks,
      std::vector<std::string> comments,
      std::vector<std::string> object_info) = 0;

  // If implemented, allows property conversion errors to be observed so that
  // the std::error_code returned from `ReadFrom` can be replaced with one that
  // is domain specific. Returning an `std::error_code` with a zero value will
  // cause the generic error to be returned.
  virtual std::error_code OnConversionFailure(const std::string& element,
                                              const std::string& property,
                                              ConversionFailureReason reason) {
    return std::error_code();
  }
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_READER_
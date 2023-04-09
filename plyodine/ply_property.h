#ifndef _PLYODINE_PLY_PROPERTY_
#define _PLYODINE_PLY_PROPERTY_

#include <cstdint>
#include <span>

namespace plyodine {

typedef int8_t Int8Property;
typedef std::span<const int8_t> Int8PropertyList;
typedef uint8_t UInt8Property;
typedef std::span<const uint8_t> UInt8PropertyList;
typedef int16_t Int16Property;
typedef std::span<const int16_t> Int16PropertyList;
typedef uint16_t UInt16Property;
typedef std::span<const uint16_t> UInt16PropertyList;
typedef int32_t Int32Property;
typedef std::span<const int32_t> Int32PropertyList;
typedef uint32_t UInt32Property;
typedef std::span<const uint32_t> UInt32PropertyList;
typedef float FloatProperty;
typedef std::span<const float> FloatPropertyList;
typedef double DoubleProperty;
typedef std::span<const double> DoublePropertyList;

}  // namespace plyodine

#endif  // _PLYODINE_PLY_PROPERTY_
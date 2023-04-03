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

enum class PropertyType {
  INT8 = 0u,
  INT8_LIST = 1u,
  UINT8 = 2u,
  UINT8_LIST = 3u,
  INT16 = 4u,
  INT16_LIST = 5u,
  UINT16 = 6u,
  UINT16_LIST = 7u,
  INT32 = 8u,
  INT32_LIST = 9u,
  UINT32 = 10u,
  UINT32_LIST = 11u,
  FLOAT = 12u,
  FLOAT_LIST = 13u,
  DOUBLE = 14u,
  DOUBLE_LIST = 15u
};

}  // namespace plyodine

#endif  // _PLYODINE_PLY_PROPERTY_
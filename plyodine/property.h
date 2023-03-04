#ifndef _PLYODINE_PROPERTY_
#define _PLYODINE_PROPERTY_

#include <cstdint>
#include <span>
#include <variant>

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

struct Property final
    : public std::variant<Int8Property, Int8PropertyList, UInt8Property,
                          UInt8PropertyList, Int16Property, Int16PropertyList,
                          UInt16Property, UInt16PropertyList, Int32Property,
                          Int32PropertyList, UInt32Property, UInt32PropertyList,
                          FloatProperty, FloatPropertyList, DoubleProperty,
                          DoublePropertyList> {
  using std::variant<Int8Property, Int8PropertyList, UInt8Property,
                     UInt8PropertyList, Int16Property, Int16PropertyList,
                     UInt16Property, UInt16PropertyList, Int32Property,
                     Int32PropertyList, UInt32Property, UInt32PropertyList,
                     FloatProperty, FloatPropertyList, DoubleProperty,
                     DoublePropertyList>::variant;

  enum Type {
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

  constexpr Type type() const { return static_cast<Type>(index()); }
};

static_assert(Property(Int8Property()).type() ==
              static_cast<size_t>(Property::Type::INT8));
static_assert(Property(Int8PropertyList()).type() ==
              static_cast<size_t>(Property::Type::INT8_LIST));
static_assert(Property(UInt8Property()).type() ==
              static_cast<size_t>(Property::Type::UINT8));
static_assert(Property(UInt8PropertyList()).type() ==
              static_cast<size_t>(Property::Type::UINT8_LIST));
static_assert(Property(Int16Property()).type() ==
              static_cast<size_t>(Property::Type::INT16));
static_assert(Property(Int16PropertyList()).type() ==
              static_cast<size_t>(Property::Type::INT16_LIST));
static_assert(Property(UInt16Property()).type() ==
              static_cast<size_t>(Property::Type::UINT16));
static_assert(Property(UInt16PropertyList()).type() ==
              static_cast<size_t>(Property::Type::UINT16_LIST));
static_assert(Property(Int32Property()).type() ==
              static_cast<size_t>(Property::Type::INT32));
static_assert(Property(Int32PropertyList()).type() ==
              static_cast<size_t>(Property::Type::INT32_LIST));
static_assert(Property(UInt32Property()).type() ==
              static_cast<size_t>(Property::Type::UINT32));
static_assert(Property(UInt32PropertyList()).type() ==
              static_cast<size_t>(Property::Type::UINT32_LIST));
static_assert(Property(FloatProperty()).type() ==
              static_cast<size_t>(Property::Type::FLOAT));
static_assert(Property(FloatPropertyList()).type() ==
              static_cast<size_t>(Property::Type::FLOAT_LIST));
static_assert(Property(DoubleProperty()).type() ==
              static_cast<size_t>(Property::Type::DOUBLE));
static_assert(Property(DoublePropertyList()).type() ==
              static_cast<size_t>(Property::Type::DOUBLE_LIST));

}  // namespace plyodine

#endif  // _PLYODINE_PROPERTY_
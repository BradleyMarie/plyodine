#ifndef _PLYODINE_PLY_PROPERTY_
#define _PLYODINE_PLY_PROPERTY_

#include <cstdint>
#include <span>
#include <variant>

namespace plyodine {

class Property final
    : public std::variant<
          std::span<const int8_t>, std::span<const std::span<const int8_t>>,
          std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
          std::span<const int16_t>, std::span<const std::span<const int16_t>>,
          std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
          std::span<const int32_t>, std::span<const std::span<const int32_t>>,
          std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
          std::span<const float>, std::span<const std::span<const float>>,
          std::span<const double>, std::span<const std::span<const double>>> {
 public:
  using std::variant<
      std::span<const int8_t>, std::span<const std::span<const int8_t>>,
      std::span<const uint8_t>, std::span<const std::span<const uint8_t>>,
      std::span<const int16_t>, std::span<const std::span<const int16_t>>,
      std::span<const uint16_t>, std::span<const std::span<const uint16_t>>,
      std::span<const int32_t>, std::span<const std::span<const int32_t>>,
      std::span<const uint32_t>, std::span<const std::span<const uint32_t>>,
      std::span<const float>, std::span<const std::span<const float>>,
      std::span<const double>,
      std::span<const std::span<const double>>>::variant;

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

static_assert(Property(std::span<const int8_t>()).type() ==
              static_cast<size_t>(Property::Type::INT8));
static_assert(Property(std::span<const std::span<const int8_t>>()).type() ==
              static_cast<size_t>(Property::Type::INT8_LIST));
static_assert(Property(std::span<const uint8_t>()).type() ==
              static_cast<size_t>(Property::Type::UINT8));
static_assert(Property(std::span<const std::span<const uint8_t>>()).type() ==
              static_cast<size_t>(Property::Type::UINT8_LIST));
static_assert(Property(std::span<const int16_t>()).type() ==
              static_cast<size_t>(Property::Type::INT16));
static_assert(Property(std::span<const std::span<const int16_t>>()).type() ==
              static_cast<size_t>(Property::Type::INT16_LIST));
static_assert(Property(std::span<const uint16_t>()).type() ==
              static_cast<size_t>(Property::Type::UINT16));
static_assert(Property(std::span<const std::span<const uint16_t>>()).type() ==
              static_cast<size_t>(Property::Type::UINT16_LIST));
static_assert(Property(std::span<const int32_t>()).type() ==
              static_cast<size_t>(Property::Type::INT32));
static_assert(Property(std::span<const std::span<const int32_t>>()).type() ==
              static_cast<size_t>(Property::Type::INT32_LIST));
static_assert(Property(std::span<const uint32_t>()).type() ==
              static_cast<size_t>(Property::Type::UINT32));
static_assert(Property(std::span<const std::span<const uint32_t>>()).type() ==
              static_cast<size_t>(Property::Type::UINT32_LIST));
static_assert(Property(std::span<const float>()).type() ==
              static_cast<size_t>(Property::Type::FLOAT));
static_assert(Property(std::span<const std::span<const float>>()).type() ==
              static_cast<size_t>(Property::Type::FLOAT_LIST));
static_assert(Property(std::span<const double>()).type() ==
              static_cast<size_t>(Property::Type::DOUBLE));
static_assert(Property(std::span<const std::span<const double>>()).type() ==
              static_cast<size_t>(Property::Type::DOUBLE_LIST));

}  // namespace plyodine

#endif  // _PLYODINE_PLY_PROPERTY_
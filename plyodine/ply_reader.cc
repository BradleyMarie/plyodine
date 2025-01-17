#include "plyodine/ply_reader.h"

#include <array>
#include <bit>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <ios>
#include <map>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "plyodine/ply_header_reader.h"

namespace {

enum class ErrorCode {
  MIN_VALUE = 1,
  BAD_STREAM = 1,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_CHAR_PROPERTY_LIST = 2,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST = 3,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_SHORT_PROPERTY_LIST = 4,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_USHORT_PROPERTY_LIST = 5,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_INT_PROPERTY_LIST = 6,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_UINT_PROPERTY_LIST = 7,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY = 8,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST = 9,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY = 10,
  INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 11,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_CHAR_PROPERTY = 12,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY = 13,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_SHORT_PROPERTY = 14,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_USHORT_PROPERTY = 15,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_INT_PROPERTY = 16,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UINT_PROPERTY = 17,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY = 18,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 19,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 20,
  INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 21,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_CHAR_PROPERTY_LIST = 22,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST = 23,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_SHORT_PROPERTY_LIST = 24,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_USHORT_PROPERTY_LIST = 25,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_INT_PROPERTY_LIST = 26,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_UINT_PROPERTY_LIST = 27,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY = 28,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST = 29,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY = 30,
  INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 31,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_CHAR_PROPERTY = 32,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY = 33,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_SHORT_PROPERTY = 34,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_USHORT_PROPERTY = 35,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_INT_PROPERTY = 36,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UINT_PROPERTY = 37,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY = 38,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 39,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 40,
  INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 41,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_CHAR_PROPERTY_LIST = 42,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST = 43,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_SHORT_PROPERTY_LIST = 44,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_USHORT_PROPERTY_LIST = 45,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_INT_PROPERTY_LIST = 46,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_UINT_PROPERTY_LIST = 47,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY = 48,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST = 49,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY = 50,
  INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 51,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_CHAR_PROPERTY = 52,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY = 53,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_SHORT_PROPERTY = 54,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_USHORT_PROPERTY = 55,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_INT_PROPERTY = 56,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UINT_PROPERTY = 57,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY = 58,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 59,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 60,
  INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 61,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_CHAR_PROPERTY_LIST = 62,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST = 63,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_SHORT_PROPERTY_LIST = 64,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_USHORT_PROPERTY_LIST = 65,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_INT_PROPERTY_LIST = 66,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_UINT_PROPERTY_LIST = 67,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY = 68,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST = 69,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY = 70,
  INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 71,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_CHAR_PROPERTY = 72,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY = 73,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_SHORT_PROPERTY = 74,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_USHORT_PROPERTY = 75,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_INT_PROPERTY = 76,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UINT_PROPERTY = 77,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY = 78,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 79,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 80,
  INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 81,
  INVALID_CONVERSION_INT_PROPERTY_TO_CHAR_PROPERTY_LIST = 82,
  INVALID_CONVERSION_INT_PROPERTY_TO_UCHAR_PROPERTY_LIST = 83,
  INVALID_CONVERSION_INT_PROPERTY_TO_SHORT_PROPERTY_LIST = 84,
  INVALID_CONVERSION_INT_PROPERTY_TO_USHORT_PROPERTY_LIST = 85,
  INVALID_CONVERSION_INT_PROPERTY_TO_INT_PROPERTY_LIST = 86,
  INVALID_CONVERSION_INT_PROPERTY_TO_UINT_PROPERTY_LIST = 87,
  INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY = 88,
  INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY_LIST = 89,
  INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY = 90,
  INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 91,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_CHAR_PROPERTY = 92,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UCHAR_PROPERTY = 93,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_SHORT_PROPERTY = 94,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_USHORT_PROPERTY = 95,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_INT_PROPERTY = 96,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UINT_PROPERTY = 97,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY = 98,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 99,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 100,
  INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 101,
  INVALID_CONVERSION_UINT_PROPERTY_TO_CHAR_PROPERTY_LIST = 102,
  INVALID_CONVERSION_UINT_PROPERTY_TO_UCHAR_PROPERTY_LIST = 103,
  INVALID_CONVERSION_UINT_PROPERTY_TO_SHORT_PROPERTY_LIST = 104,
  INVALID_CONVERSION_UINT_PROPERTY_TO_USHORT_PROPERTY_LIST = 105,
  INVALID_CONVERSION_UINT_PROPERTY_TO_INT_PROPERTY_LIST = 106,
  INVALID_CONVERSION_UINT_PROPERTY_TO_UINT_PROPERTY_LIST = 107,
  INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY = 108,
  INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY_LIST = 109,
  INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY = 110,
  INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 111,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_CHAR_PROPERTY = 112,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UCHAR_PROPERTY = 113,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_SHORT_PROPERTY = 114,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_USHORT_PROPERTY = 115,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_INT_PROPERTY = 116,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UINT_PROPERTY = 117,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY = 118,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST = 119,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 120,
  INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST = 121,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY = 122,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY_LIST = 123,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY = 124,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY_LIST = 125,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY = 126,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY_LIST = 127,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY = 128,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY_LIST = 129,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY = 130,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY_LIST = 131,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY = 132,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY_LIST = 133,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_FLOAT_PROPERTY_LIST = 134,
  INVALID_CONVERSION_FLOAT_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 135,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY = 136,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST = 137,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY = 138,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST = 139,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY = 140,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST = 141,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY = 142,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST = 143,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY = 144,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY_LIST = 145,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY = 146,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY_LIST = 147,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_FLOAT_PROPERTY = 148,
  INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 149,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY = 150,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY_LIST = 151,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY = 152,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY_LIST = 153,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY = 154,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY_LIST = 155,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY = 156,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY_LIST = 157,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY = 158,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY_LIST = 159,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY = 160,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY_LIST = 161,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_FLOAT_PROPERTY_LIST = 162,
  INVALID_CONVERSION_DOUBLE_PROPERTY_TO_DOUBLE_PROPERTY_LIST = 163,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY = 164,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST = 165,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY = 166,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST = 167,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY = 168,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST = 169,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY = 170,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST = 171,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY = 172,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY_LIST = 173,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY = 174,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY_LIST = 175,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_FLOAT_PROPERTY = 176,
  INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_DOUBLE_PROPERTY = 177,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_CHAR = 178,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UCHAR = 179,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_SHORT = 180,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_USHORT = 181,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_INT = 182,
  UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT = 183,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_CHAR = 184,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UCHAR = 185,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_SHORT = 186,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_USHORT = 187,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_INT = 188,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UINT = 189,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_FLOAT = 190,
  UNEXPECTED_EOF_PROPERTY_LIST_VALUE_DOUBLE = 191,
  UNEXPECTED_EOF_PROPERTY_VALUE_CHAR = 192,
  UNEXPECTED_EOF_PROPERTY_VALUE_UCHAR = 193,
  UNEXPECTED_EOF_PROPERTY_VALUE_SHORT = 194,
  UNEXPECTED_EOF_PROPERTY_VALUE_USHORT = 195,
  UNEXPECTED_EOF_PROPERTY_VALUE_INT = 196,
  UNEXPECTED_EOF_PROPERTY_VALUE_UINT = 197,
  UNEXPECTED_EOF_PROPERTY_VALUE_FLOAT = 198,
  UNEXPECTED_EOF_PROPERTY_VALUE_DOUBLE = 199,
  ASCII_PROPERTY_LIST_SIZE_MISSING_CHAR = 200,
  ASCII_PROPERTY_LIST_SIZE_MISSING_UCHAR = 201,
  ASCII_PROPERTY_LIST_SIZE_MISSING_SHORT = 202,
  ASCII_PROPERTY_LIST_SIZE_MISSING_USHORT = 203,
  ASCII_PROPERTY_LIST_SIZE_MISSING_INT = 204,
  ASCII_PROPERTY_LIST_SIZE_MISSING_UINT = 205,
  ASCII_PROPERTY_LIST_VALUE_MISSING_CHAR = 206,
  ASCII_PROPERTY_LIST_VALUE_MISSING_UCHAR = 207,
  ASCII_PROPERTY_LIST_VALUE_MISSING_SHORT = 208,
  ASCII_PROPERTY_LIST_VALUE_MISSING_USHORT = 209,
  ASCII_PROPERTY_LIST_VALUE_MISSING_INT = 210,
  ASCII_PROPERTY_LIST_VALUE_MISSING_UINT = 211,
  ASCII_PROPERTY_LIST_VALUE_MISSING_FLOAT = 212,
  ASCII_PROPERTY_LIST_VALUE_MISSING_DOUBLE = 213,
  ASCII_PROPERTY_VALUE_MISSING_CHAR = 214,
  ASCII_PROPERTY_VALUE_MISSING_UCHAR = 215,
  ASCII_PROPERTY_VALUE_MISSING_SHORT = 216,
  ASCII_PROPERTY_VALUE_MISSING_USHORT = 217,
  ASCII_PROPERTY_VALUE_MISSING_INT = 218,
  ASCII_PROPERTY_VALUE_MISSING_UINT = 219,
  ASCII_PROPERTY_VALUE_MISSING_FLOAT = 220,
  ASCII_PROPERTY_VALUE_MISSING_DOUBLE = 221,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_CHAR = 222,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UCHAR = 223,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_SHORT = 224,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_USHORT = 225,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_INT = 226,
  ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UINT = 227,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_CHAR = 228,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_UCHAR = 229,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_SHORT = 230,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_USHORT = 231,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_INT = 232,
  PROPERTY_LIST_SIZE_OUT_OF_RANGE_UINT = 233,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_CHAR = 234,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UCHAR = 235,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_SHORT = 236,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_USHORT = 237,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_INT = 238,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UINT = 239,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_FLOAT = 240,
  ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_DOUBLE = 241,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_CHAR = 242,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UCHAR = 243,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_SHORT = 244,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_USHORT = 245,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_INT = 246,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UINT = 247,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_FLOAT = 248,
  ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_DOUBLE = 249,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_CHAR = 250,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UCHAR = 251,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_SHORT = 252,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_USHORT = 253,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_INT = 254,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UINT = 255,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_FLOAT = 256,
  ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_DOUBLE = 257,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_CHAR = 258,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UCHAR = 259,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_SHORT = 260,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_USHORT = 261,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_INT = 262,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UINT = 263,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_FLOAT = 264,
  ASCII_PROPERTY_VALUE_OUT_OF_RANGE_DOUBLE = 265,
  OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UCHAR = 266,
  OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT = 267,
  OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_USHORT = 268,
  OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT = 269,
  OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UINT = 270,
  OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT = 271,
  OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_USHORT = 272,
  OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT = 273,
  OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_UINT = 274,
  OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_USHORT = 275,
  OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT = 276,
  OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_UINT = 277,
  OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT = 278,
  OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_UINT = 279,
  OVERFLOWED_PROPERTY_VALUE_INT_FROM_UINT = 280,
  OVERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE = 281,
  OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UCHAR = 282,
  OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT = 283,
  OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_USHORT = 284,
  OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT = 285,
  OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UINT = 286,
  OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT = 287,
  OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_USHORT = 288,
  OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT = 289,
  OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_UINT = 290,
  OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_USHORT = 291,
  OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT = 292,
  OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_UINT = 293,
  OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT = 294,
  OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_UINT = 295,
  OVERFLOWED_PROPERTY_LIST_VALUE_INT_FROM_UINT = 296,
  OVERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE = 297,
  UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT = 298,
  UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT = 299,
  UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_CHAR = 300,
  UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT = 301,
  UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT = 302,
  UNDERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT = 303,
  UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_CHAR = 304,
  UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_SHORT = 305,
  UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT = 306,
  UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_CHAR = 307,
  UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_SHORT = 308,
  UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_INT = 309,
  UNDERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE = 310,
  UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT = 311,
  UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT = 312,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_CHAR = 313,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT = 314,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT = 315,
  UNDERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT = 316,
  UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_CHAR = 317,
  UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_SHORT = 318,
  UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT = 319,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_CHAR = 320,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_SHORT = 321,
  UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_INT = 322,
  UNDERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE = 323,
  ASCII_UNUSED_TOKEN = 324,
  ASCII_MISMATCHED_LINE_ENDINGS = 325,
  ASCII_EMPTY_TOKEN = 326,
  UNEXPECTED_EOF_NO_PROPERTIES = 327,
  MAX_VALUE = 327,
};

static class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
  std::error_condition default_error_condition(int value) const noexcept;
} kErrorCategory;

const char* ErrorCategory::name() const noexcept {
  return "plyodine::PlyReader";
}

std::string ErrorCategory::message(int condition) const {
  static const std::string float_min =
      std::to_string(std::numeric_limits<float>::lowest());
  static const std::string float_max =
      std::to_string(std::numeric_limits<float>::max());
  static const std::string double_min =
      std::to_string(std::numeric_limits<double>::lowest());
  static const std::string double_max =
      std::to_string(std::numeric_limits<double>::max());

  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "The stream was not in 'good' state";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'char' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'float'";
    case ErrorCode::
        INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'char' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uchar' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'float'";
    case ErrorCode::
        INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uchar' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'short' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'float'";
    case ErrorCode::
        INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'short' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'ushort' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'float'";
    case ErrorCode::
        INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'ushort' to property list of data type "
             "'double'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'int' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'int' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'uint' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'float'";
    case ErrorCode::
        INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property of type 'double'";
    case ErrorCode::
        INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'uint' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'float' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'char'";
    case ErrorCode::
        INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'uchar'";
    case ErrorCode::
        INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'short'";
    case ErrorCode::
        INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'ushort'";
    case ErrorCode::
        INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'uint'";
    case ErrorCode::
        INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'float' to property of type 'double'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'char'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'short'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'ushort'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'int'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property of type 'uint'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_FLOAT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'float'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_DOUBLE_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property of "
             "type 'double' to property list of data type 'double'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'char'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type 'char'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'uchar'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type 'uchar'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'short'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type 'short'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'ushort'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type "
             "'ushort'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'int'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type 'int'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'uint'";
    case ErrorCode::
        INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY_LIST:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property list of data type 'uint'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_FLOAT_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'float'";
    case ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_DOUBLE_PROPERTY:
      return "A callback requested an unsupported conversion from property "
             "list of data type 'double' to property of type 'double'";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_CHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list size of type 'char')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UCHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list size of type 'uchar')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_SHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list size of type 'short')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_USHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list size of type 'ushort')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_INT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list size of type 'int')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT:
      return "The stream ended earlier than than expected (reached EOF but "
             "expected to find a property list size of type 'uint')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_CHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'char')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UCHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'uchar')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_SHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'short')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_USHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'ushort')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_INT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'int')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UINT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'uint')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_FLOAT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'float')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_DOUBLE:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property list value of type 'double')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_CHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'char')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UCHAR:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'uchar')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_SHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'short')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_USHORT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'ushort')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_INT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'int')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UINT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'uint')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_FLOAT:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'float')";
    case ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_DOUBLE:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find a property value of type 'double')";
    case ErrorCode::UNEXPECTED_EOF_NO_PROPERTIES:
      return "The stream ended earlier than expected (reached EOF but expected "
             "to find an element with no properties')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_CHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type "
             "'char')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UCHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type "
             "'uchar')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_SHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type "
             "'short')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_USHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type "
             "'ushort')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_INT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type 'int')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UINT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list size of type "
             "'uint')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_CHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'char')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_UCHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'uchar')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_SHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'short')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_USHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'ushort')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_INT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'int')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_UINT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'uint')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_FLOAT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'float')";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_DOUBLE:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property list value of type "
             "'double')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_CHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'char')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_UCHAR:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'uchar')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_SHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'short')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_USHORT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'ushort')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_INT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'int')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_UINT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'uint')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_FLOAT:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'float')";
    case ErrorCode::ASCII_PROPERTY_VALUE_MISSING_DOUBLE:
      return "A line in the input had fewer tokens than expected (reached end "
             "of line but expected to find a property value of type 'double')";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_CHAR:
      return "A property list with size type 'char' had a size that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UCHAR:
      return "A property list with size type 'uchar' had a size that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_SHORT:
      return "A property list with size type 'short' had a size that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_USHORT:
      return "A property list with size type 'ushort' had a size that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_INT:
      return "A property list with size type 'int' had a size that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UINT:
      return "A property list with size type 'uint' had a size that could not "
             "be parsed";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_CHAR:
      return "A property list with size type 'char' had a size that was out of "
             "range (must have between 0 and 127 entries)";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UCHAR:
      return "A property list with size type 'uchar' had a size that was out "
             "of range (must have between 0 and 255 entries)";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_SHORT:
      return "A property list with size type 'short' had a size that was out "
             "of range (must have between 0 and 32,767 entries)";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_USHORT:
      return "A property list with size type 'ushort' had a size that was out "
             "of range (must have between 0 and 65,535 entries)";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_INT:
      return "A property list with size type 'int' had a size that was out of "
             "range (must have between 0 and 2,147,483,647 entries)";
    case ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UINT:
      return "A property list with size type 'uint' had a size that was out of "
             "range (must have between 0 and 4,294,967,295 entries)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_CHAR:
      return "A property list with data type 'char' had a value that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UCHAR:
      return "A property list with data type 'uchar' had a value that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_SHORT:
      return "A property list with data type 'short' had a value that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_USHORT:
      return "A property list with data type 'ushort' had a value that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_INT:
      return "A property list with data type 'int' had a value that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UINT:
      return "A property list with data type 'uint' had a value that could not "
             "be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_FLOAT:
      return "A property list with data type 'float' had a value that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_DOUBLE:
      return "A property list with data type 'double' had a value that could "
             "not be parsed";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_CHAR:
      return "A property list with data type 'char' had a value that was out "
             "of range (must be between -128 and 127)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UCHAR:
      return "A property list with data type 'uchar' had a value that was out "
             "of range (must be between 0 and 255)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_SHORT:
      return "A property list with data type 'short' had a value that was out "
             "of range (must be between -32,768 and 32,767)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_USHORT:
      return "A property list with data type 'ushort' had a value that was out "
             "of range (must be between 0 and 65,535)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_INT:
      return "A property list with data type 'int' had a value that was out of "
             "range (must be between -2,147,483,648 and 2,147,483,647)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UINT:
      return "A property list with data type 'uint' had a value that was out "
             "of range (must be between 0 and 4,294,967,295)";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_FLOAT:
      return "A property list with data type 'float' had a value that was out "
             "of range (must be between ~" +
             float_min + " and ~" + float_max + ")";
    case ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_DOUBLE:
      return "A property list with data type 'double' had a value that was out "
             "of range (must be between ~" +
             double_min + " and ~" + double_max + ")";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_CHAR:
      return "A property with type 'char' had a value that could not be parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UCHAR:
      return "A property with type 'uchar' had a value that could not be "
             "parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_SHORT:
      return "A property with type 'short' had a value that could not be "
             "parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_USHORT:
      return "A property with type 'ushort' had a value that could not be "
             "parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_INT:
      return "A property with type 'int' had a value that could not be parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UINT:
      return "A property with type 'uint' had a value that could not be parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_FLOAT:
      return "A property with type 'float' had a value that could not be "
             "parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_DOUBLE:
      return "A property with type 'double' had a value that could not be "
             "parsed";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_CHAR:
      return "A property with data type 'char' had a value that was out of "
             "range (must be between -128 and 127)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UCHAR:
      return "A property with data type 'uchar' had a value that was out of "
             "range (must be between 0 and 255)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_SHORT:
      return "A property with data type 'short' had a value that was out of "
             "range (must be between -32,768 and 32,767)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_USHORT:
      return "A property with data type 'ushort' had a value that was out of "
             "range (must be between 0 and 65,535)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_INT:
      return "A property with data type 'int' had a value that was out of "
             "range (must be between -2,147,483,648 and 2,147,483,647)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UINT:
      return "A property with data type 'uint' had a value that was out of "
             "range (must be between 0 and 4,294,967,295)";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_FLOAT:
      return "A property with data type 'float' had a value that was out of "
             "range (must be between ~" +
             float_min + " and ~" + float_max + ")";
    case ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_DOUBLE:
      return "A property with data type 'double' had a value that was out of "
             "range (must be between ~" +
             double_min + " and ~" + double_max + ")";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UCHAR:
      return "A conversion of a property value from type 'uchar' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_USHORT:
      return "A conversion of a property value from type 'ushort' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'char' "
             "overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UINT:
      return "A conversion of a property value from type 'uint' to type 'char' "
             "overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_USHORT:
      return "A conversion of a property value from type 'ushort' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'uchar' "
             "overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_UINT:
      return "A conversion of a property value from type 'uint' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_USHORT:
      return "A conversion of a property value from type 'ushort' to type "
             "'short' overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'short' "
             "overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_UINT:
      return "A conversion of a property value from type 'uint' to type "
             "'short' overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT:
      return "A conversion of a property value from type 'int' to type "
             "'ushort' overflowed (must be between 0 and 65,535)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_UINT:
      return "A conversion of a property value from type 'uint' to type "
             "'ushort' overflowed (must be between 0 and 65,535)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_INT_FROM_UINT:
      return "A conversion of a property value from type 'uint' to type 'int' "
             "overflowed (must be between -2,147,483,648 and 2,147,483,647)";
    case ErrorCode::OVERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE:
      return "A conversion of a property value from type 'double' to type "
             "'float' overflowed (must be between " +
             float_min + " and " + float_max + ")";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UCHAR:
      return "A conversion of a property list value from type 'uchar' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_USHORT:
      return "A conversion of a property list value from type 'ushort' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UINT:
      return "A conversion of a property list value from type 'uint' to type "
             "'char' overflowed (must be between -128 and 127)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_USHORT:
      return "A conversion of a property list value from type 'ushort' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_UINT:
      return "A conversion of a property list value from type 'uint' to type "
             "'uchar' overflowed (must be between 0 and 255)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_USHORT:
      return "A conversion of a property list value from type 'ushort' to type "
             "'short' overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'short' overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_UINT:
      return "A conversion of a property list value from type 'uint' to type "
             "'short' overflowed (must be between -32,768 and 32,767)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'ushort' overflowed (must be between 0 and 65,535)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_UINT:
      return "A conversion of a property list value from type 'uint' to type "
             "'ushort' overflowed (must be between 0 and 65,535)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_INT_FROM_UINT:
      return "A conversion of a property list value from type 'uint' to type "
             "'int' overflowed (must be between -2,147,483,648 and "
             "2,147,483,647)";
    case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE:
      return "A conversion of a property list value from type 'double' to type "
             "'float' overflowed (must be between " +
             float_min + " and " + float_max + ")";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'char' underflowed (must be between -128 and 127)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'char' "
             "underflowed (must be between -128 and 127)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_CHAR:
      return "A conversion of a property value from type 'char' to type "
             "'uchar' underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'uchar' underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'uchar' "
             "underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'short' "
             "underflowed (must be between -32,768 and 32,767)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_CHAR:
      return "A conversion of a property value from type 'char' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT:
      return "A conversion of a property value from type 'int' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_CHAR:
      return "A conversion of a property value from type 'char' to type 'uint' "
             "underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_SHORT:
      return "A conversion of a property value from type 'short' to type "
             "'uint' underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_INT:
      return "A conversion of a property value from type 'int' to type 'uint' "
             "underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE:
      return "A conversion of a property value from type 'double' to type "
             "'float' underflowed (must be between " +
             float_min + " and " + float_max + ")";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'char' underflowed (must be between -128 and 127)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'char' underflowed (must be between -128 and 127)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_CHAR:
      return "A conversion of a property list value from type 'char' to type "
             "'uchar' underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'uchar' underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'uchar' underflowed (must be between 0 and 255)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'short' underflowed (must be between -32,768 and 32,767)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_CHAR:
      return "A conversion of a property list value from type 'char' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'ushort' underflowed (must be between 0 and 65,535)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_CHAR:
      return "A conversion of a property list value from type 'char' to type "
             "'uint' underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_SHORT:
      return "A conversion of a property list value from type 'short' to type "
             "'uint' underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_INT:
      return "A conversion of a property list value from type 'int' to type "
             "'uint' underflowed (must be between 0 and 4,294,967,295)";
    case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE:
      return "A conversion of a property list value from type 'double' to type "
             "'float' underflowed (must be between " +
             float_min + " and " + float_max + ")";
    case ErrorCode::ASCII_UNUSED_TOKEN:
      return "The input contained a data token that was not associated with "
             "any property";
    case ErrorCode::ASCII_MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::ASCII_EMPTY_TOKEN:
      return "The input contained an empty token (tokens on non-comment lines "
             "must be separated by exactly one ASCII space with no leading or "
             "trailing whitespace on the line)";
  };

  return "Unknown Error";
}

std::error_condition ErrorCategory::default_error_condition(
    int value) const noexcept {
  if (value < static_cast<int>(ErrorCode::MIN_VALUE) ||
      value > static_cast<int>(ErrorCode::MAX_VALUE)) {
    return std::error_condition(value, *this);
  }

  return std::make_error_condition(std::errc::invalid_argument);
}

std::error_code make_error_code(ErrorCode code) {
  return std::error_code(static_cast<int>(code), kErrorCategory);
}

}  // namespace

namespace std {

template <>
struct is_error_code_enum<ErrorCode> : true_type {};

}  // namespace std

namespace plyodine {
namespace {

enum class ReadType {
  PROPERTY_LIST_SIZE = 0,
  PROPERTY_LIST_VALUE = 1,
  PROPERTY_VALUE = 2,
};

using ContextData =
    std::tuple<int8_t, std::vector<int8_t>, uint8_t, std::vector<uint8_t>,
               int16_t, std::vector<int16_t>, uint16_t, std::vector<uint16_t>,
               int32_t, std::vector<int32_t>, uint32_t, std::vector<uint32_t>,
               float, std::vector<float>, double, std::vector<double>>;

struct Context final {
  ContextData data;
  std::string_view line_ending;
  std::stringstream line;
  std::string token;
  bool eof = false;
};

using AppendFunc = void (*)(Context&);
using ConvertFunc = std::error_code (*)(Context&, ReadType);
using Handler = std::function<std::error_code(Context&)>;
using OnConversionErrorFunc = std::function<std::error_code(
    const std::string&, const std::string&, std::error_code)>;
using ReadFunc = std::error_code (*)(std::istream&, Context&, ReadType);

template <typename T>
consteval size_t GetTypeIndex() {
  if constexpr (std::is_same_v<T, int8_t>) {
    return 0;
  } else if constexpr (std::is_same_v<T, uint8_t>) {
    return 1;
  } else if constexpr (std::is_same_v<T, int16_t>) {
    return 2;
  } else if constexpr (std::is_same_v<T, uint16_t>) {
    return 3;
  } else if constexpr (std::is_same_v<T, int32_t>) {
    return 4;
  } else if constexpr (std::is_same_v<T, uint32_t>) {
    return 5;
  } else if constexpr (std::is_same_v<T, float>) {
    return 6;
  } else {
    return 7;
  }
}

std::error_code ReadNextLine(std::istream& stream, Context& context,
                             ErrorCode end_of_file_error) {
  std::string_view line_ending = context.line_ending;

  context.line.str("");
  context.line.clear();

  char c = 0;
  while (stream.get(c)) {
    if (c == line_ending[0]) {
      line_ending.remove_prefix(1);

      while (!line_ending.empty() && stream.get(c)) {
        if (c != line_ending[0]) {
          return ErrorCode::ASCII_MISMATCHED_LINE_ENDINGS;
        }
        line_ending.remove_prefix(1);
        context.eof = stream.eof();
      }

      break;
    }

    context.line.put(c);
  }

  if (!c) {
    return end_of_file_error;
  }

  if (stream.fail() && !stream.eof()) {
    return std::io_errc::stream;
  }

  return std::error_code();
}

std::error_code ReadNextToken(Context& context, bool allow_decimal,
                              ErrorCode unparsable_error,
                              ErrorCode missing_token_error,
                              ErrorCode end_of_line_error) {
  context.token.clear();

  char c = 0;
  while (context.line.get(c)) {
    if (c == ' ') {
      break;
    }

    if (c == '-') {
      if (!context.token.empty()) {
        return unparsable_error;
      }
    } else if (c == '.') {
      if (!allow_decimal) {
        return unparsable_error;
      }

      allow_decimal = false;
    } else if (!std::isdigit(c)) {
      return unparsable_error;
    }

    context.token.push_back(c);
  }

  if (!c) {
    return context.eof ? end_of_line_error : missing_token_error;
  }

  if (context.token.empty()) {
    return ErrorCode::ASCII_EMPTY_TOKEN;
  }

  return std::error_code();
}

consteval std::array<ErrorCode, 3> UnexpectedEofErrors(size_t index) {
  static constexpr ErrorCode property_list_size[8] = {
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_CHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UCHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_SHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_USHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_INT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT,  // Unused
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT,  // Unused
  };

  static constexpr ErrorCode property_list_value[8] = {
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_CHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UCHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_SHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_USHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_INT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_UINT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_FLOAT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_VALUE_DOUBLE,
  };

  static constexpr ErrorCode property_value[8] = {
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_CHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UCHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_SHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_USHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_INT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UINT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_FLOAT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_DOUBLE,
  };

  return {property_list_size[index], property_list_value[index],
          property_value[index]};
}

consteval std::array<ErrorCode, 3> MissingErrors(size_t index) {
  static constexpr ErrorCode property_list_size[8] = {
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_CHAR,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UCHAR,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_SHORT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_USHORT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_INT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UINT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UINT,  // Unused
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_MISSING_UINT,  // Unused
  };

  static constexpr ErrorCode property_list_value[8] = {
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_CHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_UCHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_SHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_USHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_INT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_UINT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_FLOAT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_MISSING_DOUBLE,
  };

  static constexpr ErrorCode property_value[8] = {
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_CHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_UCHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_SHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_USHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_INT,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_UINT,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_FLOAT,
      ErrorCode::ASCII_PROPERTY_VALUE_MISSING_DOUBLE,
  };

  return {property_list_size[index], property_list_value[index],
          property_value[index]};
}

consteval std::array<ErrorCode, 3> OutOfRangeErrors(size_t index) {
  static constexpr ErrorCode property_list_size[8] = {
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_CHAR,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UCHAR,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_SHORT,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_USHORT,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_INT,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UINT,
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UINT,  // Unused
      ErrorCode::PROPERTY_LIST_SIZE_OUT_OF_RANGE_UINT,  // Unused
  };

  static constexpr ErrorCode property_list_value[8] = {
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_CHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UCHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_SHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_USHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_INT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_UINT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_FLOAT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_OUT_OF_RANGE_DOUBLE,
  };

  static constexpr ErrorCode property_value[8] = {
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_CHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UCHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_SHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_USHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_INT,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_UINT,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_FLOAT,
      ErrorCode::ASCII_PROPERTY_VALUE_OUT_OF_RANGE_DOUBLE,
  };

  return {property_list_size[index], property_list_value[index],
          property_value[index]};
}

consteval std::array<ErrorCode, 3> FailedToParseErrors(size_t index) {
  static constexpr ErrorCode property_list_size[8] = {
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_CHAR,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UCHAR,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_SHORT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_USHORT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_INT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UINT,
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UINT,  // Unused
      ErrorCode::ASCII_PROPERTY_LIST_SIZE_FAILED_TO_PARSE_UINT,  // Unused
  };

  static constexpr ErrorCode property_list_value[8] = {
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_CHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UCHAR,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_SHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_USHORT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_INT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_UINT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_FLOAT,
      ErrorCode::ASCII_PROPERTY_LIST_VALUE_FAILED_TO_PARSE_DOUBLE,
  };

  static constexpr ErrorCode property_value[8] = {
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_CHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UCHAR,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_SHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_USHORT,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_INT,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_UINT,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_FLOAT,
      ErrorCode::ASCII_PROPERTY_VALUE_FAILED_TO_PARSE_DOUBLE,
  };

  return {property_list_size[index], property_list_value[index],
          property_value[index]};
}

template <typename T>
std::error_code ReadASCII(std::istream& input, Context& context,
                          ReadType read_type) {
  static constexpr std::array<ErrorCode, 3> missing =
      MissingErrors(GetTypeIndex<T>());
  static constexpr std::array<ErrorCode, 3> unexpected_eof =
      UnexpectedEofErrors(GetTypeIndex<T>());
  static constexpr std::array<ErrorCode, 3> failed_to_parse =
      FailedToParseErrors(GetTypeIndex<T>());
  static constexpr std::array<ErrorCode, 3> out_of_range =
      OutOfRangeErrors(GetTypeIndex<T>());

  if (std::error_code error =
          ReadNextToken(context, std::is_floating_point_v<T>,
                        failed_to_parse[static_cast<size_t>(read_type)],
                        missing[static_cast<size_t>(read_type)],
                        unexpected_eof[static_cast<size_t>(read_type)]);
      error) {
    return error;
  }

  T value;
  auto parsing_result = std::from_chars(
      context.token.data(), context.token.data() + context.token.size(), value);
  if (parsing_result.ec == std::errc::result_out_of_range) {
    return out_of_range[static_cast<size_t>(read_type)];
  }

  if (read_type == ReadType::PROPERTY_LIST_SIZE && value < 0) {
    return out_of_range[static_cast<size_t>(read_type)];
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::integral T>
std::error_code ReadBinary(std::istream& stream, Context& context,
                           ReadType read_type) {
  static constexpr std::array<ErrorCode, 3> unexpected_eof =
      UnexpectedEofErrors(GetTypeIndex<T>());
  static constexpr std::array<ErrorCode, 3> out_of_range =
      OutOfRangeErrors(GetTypeIndex<T>());

  T value;
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
  if (stream.fail()) {
    if (stream.eof()) {
      return unexpected_eof[static_cast<size_t>(read_type)];
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  if (read_type == ReadType::PROPERTY_LIST_SIZE && value < 0) {
    return out_of_range[static_cast<size_t>(read_type)];
  }

  std::get<T>(context.data) = value;

  return std::error_code();
}

template <std::endian Endianness, std::floating_point T>
std::error_code ReadBinary(std::istream& stream, Context& context,
                           ReadType read_type) {
  static constexpr std::array<ErrorCode, 3> unexpected_eof =
      UnexpectedEofErrors(GetTypeIndex<T>());
  static constexpr std::array<ErrorCode, 3> out_of_range =
      OutOfRangeErrors(GetTypeIndex<T>());

  std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t> value;

  stream.read(reinterpret_cast<char*>(&value), sizeof(value));
  if (stream.fail()) {
    if (stream.eof()) {
      return unexpected_eof[static_cast<size_t>(read_type)];
    }
    return std::io_errc::stream;
  }

  if (Endianness != std::endian::native) {
    value = std::byteswap(value);
  }

  if (read_type == ReadType::PROPERTY_LIST_SIZE && value < 0) {
    return out_of_range[static_cast<size_t>(read_type)];
  }

  std::get<T>(context.data) = std::bit_cast<T>(value);

  return std::error_code();
}

ReadFunc GetReadFunc(PlyHeader::Format format, PlyHeader::Property::Type type) {
  static constexpr ReadFunc ascii_read_funcs[8] = {
      ReadASCII<std::tuple_element_t<0, ContextData>>,
      ReadASCII<std::tuple_element_t<2, ContextData>>,
      ReadASCII<std::tuple_element_t<4, ContextData>>,
      ReadASCII<std::tuple_element_t<6, ContextData>>,
      ReadASCII<std::tuple_element_t<8, ContextData>>,
      ReadASCII<std::tuple_element_t<10, ContextData>>,
      ReadASCII<std::tuple_element_t<12, ContextData>>,
      ReadASCII<std::tuple_element_t<14, ContextData>>,
  };

  static constexpr ReadFunc big_endian_read_funcs[8] = {
      ReadBinary<std::endian::big, std::tuple_element_t<0, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<2, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<4, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<6, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<8, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<10, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<12, ContextData>>,
      ReadBinary<std::endian::big, std::tuple_element_t<14, ContextData>>,
  };

  static constexpr ReadFunc little_endian_read_funcs[8] = {
      ReadBinary<std::endian::little, std::tuple_element_t<0, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<2, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<4, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<6, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<8, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<10, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<12, ContextData>>,
      ReadBinary<std::endian::little, std::tuple_element_t<14, ContextData>>,
  };

  if (format == PlyHeader::Format::ASCII) {
    return ascii_read_funcs[static_cast<size_t>(type)];
  }

  if (format == PlyHeader::Format::BINARY_BIG_ENDIAN) {
    return big_endian_read_funcs[static_cast<size_t>(type)];
  }

  return little_endian_read_funcs[static_cast<size_t>(type)];
}

template <typename Source, typename Dest>
ErrorCode UnderflowError(ReadType read_type) {
  if (read_type == ReadType::PROPERTY_VALUE) {
    if constexpr (std::is_same_v<Dest, int8_t>) {
      if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, uint8_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, int16_t>) {
      static_assert(std::is_same_v<Source, int32_t>);
      return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT;
    } else if constexpr (std::is_same_v<Dest, uint16_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, uint32_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_INT;
      }
    } else {
      static_assert(std::is_same_v<Dest, float> &&
                    std::is_same_v<Source, double>);
      return ErrorCode::UNDERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE;
    }
  } else {
    if constexpr (std::is_same_v<Dest, int8_t>) {
      if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, uint8_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, int16_t>) {
      static_assert(std::is_same_v<Source, int32_t>);
      return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT;
    } else if constexpr (std::is_same_v<Dest, uint16_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT;
      }
    } else if constexpr (std::is_same_v<Dest, uint32_t>) {
      if constexpr (std::is_same_v<Source, int8_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_CHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_SHORT;
      } else {
        static_assert(std::is_same_v<Source, int32_t>);
        return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_INT;
      }
    } else {
      static_assert(std::is_same_v<Dest, float> &&
                    std::is_same_v<Source, double>);
      return ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE;
    }
  }
}

template <typename Source, typename Dest>
ErrorCode OverflowedError(ReadType read_type) {
  if (read_type == ReadType::PROPERTY_VALUE) {
    if constexpr (std::is_same_v<Dest, int8_t>) {
      if constexpr (std::is_same_v<Source, uint8_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UCHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT;
      } else if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_USHORT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, uint8_t>) {
      if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT;
      } else if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_USHORT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, int16_t>) {
      if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_UINT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, uint16_t>) {
      if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, int32_t>) {
      static_assert(std::is_same_v<Source, uint32_t>);
      return ErrorCode::OVERFLOWED_PROPERTY_VALUE_INT_FROM_UINT;
    } else {
      static_assert(std::is_same_v<Dest, float> &&
                    std::is_same_v<Source, double>);
      return ErrorCode::OVERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE;
    }
  } else {
    if constexpr (std::is_same_v<Dest, int8_t>) {
      if constexpr (std::is_same_v<Source, uint8_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UCHAR;
      } else if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT;
      } else if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_USHORT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, uint8_t>) {
      if constexpr (std::is_same_v<Source, int16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT;
      } else if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_USHORT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, int16_t>) {
      if constexpr (std::is_same_v<Source, uint16_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_UINT;
      } else if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, uint16_t>) {
      if constexpr (std::is_same_v<Source, int32_t>) {
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT;
      } else {
        static_assert(std::is_same_v<Source, uint32_t>);
        return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_UINT;
      }
    } else if constexpr (std::is_same_v<Dest, int32_t>) {
      static_assert(std::is_same_v<Source, uint32_t>);
      return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_INT_FROM_UINT;
    } else {
      static_assert(std::is_same_v<Dest, float> &&
                    std::is_same_v<Source, double>);
      return ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE;
    }
  }
}

template <typename Source, typename Dest>
std::error_code Convert(Context& context, ReadType read_type) {
  static_assert(std::is_floating_point_v<Source> ==
                std::is_floating_point_v<Dest>);

  if constexpr (!std::is_same_v<Source, Dest>) {
    if constexpr (std::is_floating_point_v<Source>) {
      if constexpr (sizeof(Dest) < sizeof(Source)) {
        if (std::isfinite(std::get<Source>(context.data))) {
          if (std::get<Source>(context.data) <
              std::numeric_limits<Dest>::lowest()) {
            return UnderflowError<Source, Dest>(read_type);
          }

          if (std::get<Source>(context.data) >
              std::numeric_limits<Dest>::max()) {
            return OverflowedError<Source, Dest>(read_type);
          }
        }
      }
    } else {
      if constexpr (std::is_signed_v<Source> && !std::is_signed_v<Dest>) {
        if (std::get<Source>(context.data) < 0) {
          return UnderflowError<Source, Dest>(read_type);
        }
      } else if constexpr (std::is_signed_v<Source> && std::is_signed_v<Dest> &&
                           sizeof(Dest) < sizeof(Source)) {
        if (std::get<Source>(context.data) < std::numeric_limits<Dest>::min()) {
          return UnderflowError<Source, Dest>(read_type);
        }
      }

      if constexpr (sizeof(Source) > sizeof(Dest) ||
                    (sizeof(Source) == sizeof(Dest) &&
                     !std::is_signed_v<Source> && std::is_signed_v<Dest>)) {
        if (std::get<Source>(context.data) >
            static_cast<Source>(std::numeric_limits<Dest>::max())) {
          return OverflowedError<Source, Dest>(read_type);
        }
      }
    }

    std::get<Dest>(context.data) =
        static_cast<Dest>(std::get<Source>(context.data));
  }

  return std::error_code();
}

template <size_t SourceTypeIndex, size_t DestTypeIndex>
consteval ConvertFunc GetConvertFunc() {
  using Source = std::tuple_element_t<SourceTypeIndex * 2, ContextData>;
  using Dest = std::tuple_element_t<DestTypeIndex * 2, ContextData>;

  if constexpr (std::is_floating_point_v<Source> ==
                std::is_floating_point_v<Dest>) {
    return Convert<Source, Dest>;
  }

  return nullptr;
}

template <size_t SourceTypeIndex>
consteval std::array<ConvertFunc, 8> GetConvertFuncs() {
  return {
      GetConvertFunc<SourceTypeIndex, 0>(),
      GetConvertFunc<SourceTypeIndex, 1>(),
      GetConvertFunc<SourceTypeIndex, 2>(),
      GetConvertFunc<SourceTypeIndex, 3>(),
      GetConvertFunc<SourceTypeIndex, 4>(),
      GetConvertFunc<SourceTypeIndex, 5>(),
      GetConvertFunc<SourceTypeIndex, 6>(),
      GetConvertFunc<SourceTypeIndex, 7>(),
  };
}

ConvertFunc GetConvertFunc(PlyHeader::Property::Type source,
                           PlyHeader::Property::Type dest) {
  static constexpr std::array<ConvertFunc, 8> conversion_funcs[8] = {
      GetConvertFuncs<0>(), GetConvertFuncs<1>(), GetConvertFuncs<2>(),
      GetConvertFuncs<3>(), GetConvertFuncs<4>(), GetConvertFuncs<5>(),
      GetConvertFuncs<6>(), GetConvertFuncs<7>(),
  };

  return conversion_funcs[static_cast<size_t>(source)]
                         [static_cast<size_t>(dest)];
}

template <size_t Index>
void Append(Context& context) {
  std::get<2 * Index + 1>(context.data)
      .push_back(std::get<2 * Index>(context.data));
}

AppendFunc GetAppendFunc(PlyHeader::Property::Type dest_type) {
  static constexpr AppendFunc append_funcs[8] = {
      Append<0>, Append<1>, Append<2>, Append<3>,
      Append<4>, Append<5>, Append<6>, Append<7>,
  };

  return append_funcs[static_cast<size_t>(dest_type)];
}

template <typename T>
Handler MakeHandler(const std::function<std::error_code(T)>& callback) {
  return [&](Context& context) { return callback(std::get<T>(context.data)); };
}

template <typename T>
Handler MakeHandler(
    const std::function<std::error_code(std::span<const T>)>& callback) {
  return [&](Context& context) {
    auto& data = std::get<std::vector<T>>(context.data);
    std::error_code result = callback(data);
    data.clear();
    return result;
  };
}

template <typename PropertyCallback>
Handler MakeHandler(const PropertyCallback& callback) {
  return std::visit(
      [](const auto& true_callback) -> Handler {
        if (!true_callback) {
          return Handler();
        }

        return MakeHandler(true_callback);
      },
      callback);
}

class PropertyParser {
 public:
  PropertyParser(PlyHeader::Format format,
                 std::optional<PlyHeader::Property::Type> list_type,
                 PlyHeader::Property::Type source_type,
                 PlyHeader::Property::Type dest_type, Handler handler,
                 OnConversionErrorFunc on_conversion_error,
                 const std::string& element_name,
                 const std::string& property_name);

  std::error_code Parse(std::istream& stream, Context& context) const;

 private:
  const std::string& element_name_;
  const std::string& property_name_;
  ReadFunc read_length_;
  ConvertFunc convert_length_;
  ReadFunc read_;
  ConvertFunc convert_;
  AppendFunc append_to_list_;
  OnConversionErrorFunc on_conversion_error_;
  Handler handler_;
};

PropertyParser::PropertyParser(
    PlyHeader::Format format,
    std::optional<PlyHeader::Property::Type> list_type,
    PlyHeader::Property::Type source_type, PlyHeader::Property::Type dest_type,
    Handler handler, OnConversionErrorFunc on_conversion_error,
    const std::string& element_name, const std::string& property_name)
    : element_name_(element_name),
      property_name_(property_name),
      read_length_(list_type ? GetReadFunc(format, *list_type) : nullptr),
      convert_length_(
          list_type
              ? GetConvertFunc(*list_type, PlyHeader::Property::Type::UINT)
              : nullptr),
      read_(GetReadFunc(format, source_type)),
      convert_(GetConvertFunc(source_type, dest_type)),
      append_to_list_(list_type ? GetAppendFunc(dest_type) : nullptr),
      on_conversion_error_(std::move(on_conversion_error)),
      handler_(std::move(handler)) {}

std::error_code PropertyParser::Parse(std::istream& stream,
                                      Context& context) const {
  uint32_t length = 1;
  if (read_length_) {
    if (std::error_code error =
            read_length_(stream, context, ReadType::PROPERTY_LIST_SIZE);
        error) {
      return error;
    }

    convert_length_(context, ReadType::PROPERTY_LIST_SIZE);

    length = std::get<uint32_t>(context.data);
  }

  for (uint32_t i = 0; i < length; i++) {
    ReadType read_type =
        read_length_ ? ReadType::PROPERTY_LIST_VALUE : ReadType::PROPERTY_VALUE;
    if (std::error_code error = read_(stream, context, read_type); error) {
      return error;
    }

    if (std::error_code error = convert_(context, read_type); error) {
      return on_conversion_error_(element_name_, property_name_, error);
    }

    if (append_to_list_) {
      append_to_list_(context);
    }
  }

  if (handler_) {
    if (std::error_code error = handler_(context); error) {
      return error;
    }
  }

  return std::error_code();
}

template <typename PropertyCallback>
PropertyCallback MakeEmptyCallback(PlyHeader::Property::Type data_type,
                                   bool is_list) {
  static const PropertyCallback empty_callbacks[16] = {
      PropertyCallback(std::in_place_index<0>),
      PropertyCallback(std::in_place_index<1>),
      PropertyCallback(std::in_place_index<2>),
      PropertyCallback(std::in_place_index<3>),
      PropertyCallback(std::in_place_index<4>),
      PropertyCallback(std::in_place_index<5>),
      PropertyCallback(std::in_place_index<6>),
      PropertyCallback(std::in_place_index<7>),
      PropertyCallback(std::in_place_index<8>),
      PropertyCallback(std::in_place_index<9>),
      PropertyCallback(std::in_place_index<10>),
      PropertyCallback(std::in_place_index<11>),
      PropertyCallback(std::in_place_index<12>),
      PropertyCallback(std::in_place_index<13>),
      PropertyCallback(std::in_place_index<14>),
      PropertyCallback(std::in_place_index<15>)};

  return empty_callbacks[2 * static_cast<size_t>(data_type) +
                         static_cast<size_t>(is_list)];
}

static constexpr ErrorCode kProhibitedConversions[16][16] = {
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_CHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_UCHAR_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_SHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_USHORT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_INT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_UINT_PROPERTY_LIST_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_INT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_FLOAT_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
    },
    {
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_CHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_SHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_USHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_INT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_UINT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_FLOAT_PROPERTY_LIST,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_TO_DOUBLE_PROPERTY_LIST,
    },
    {
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_CHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UCHAR_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_SHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_USHORT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_INT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY,
        ErrorCode::
            INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_UINT_PROPERTY_LIST,
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_FLOAT_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
        ErrorCode::INVALID_CONVERSION_DOUBLE_PROPERTY_LIST_TO_DOUBLE_PROPERTY,
        ErrorCode::MIN_VALUE,  // Supported
    },
};

}  // namespace

std::error_code PlyReader::ReadFrom(std::istream& stream) {
  if (!stream) {
    return ErrorCode::BAD_STREAM;
  }

  auto header = ReadPlyHeader(stream);
  if (!header) {
    return header.error();
  }

  std::map<std::string, uintmax_t> num_element_instances;
  std::map<std::string, std::map<std::string, PropertyCallback>>
      actual_callbacks;
  for (const auto& element : header->elements) {
    num_element_instances[element.name] = element.num_in_file;

    std::map<std::string, PropertyCallback>& property_callbacks =
        actual_callbacks[element.name];
    for (const auto& property : element.properties) {
      property_callbacks[property.name] = MakeEmptyCallback<PropertyCallback>(
          property.data_type, property.list_type.has_value());
    }
  }

  auto requested_callbacks = actual_callbacks;
  if (std::error_code error =
          Start(std::move(num_element_instances), requested_callbacks,
                std::move(header->comments), std::move(header->object_info));
      error) {
    return error;
  }

  for (auto& [element_name, element_callbacks] : requested_callbacks) {
    auto element_iter = actual_callbacks.find(element_name);
    if (element_iter == actual_callbacks.end()) {
      continue;
    }

    for (auto& [property_name, property_callback] : element_callbacks) {
      auto property_iter = element_iter->second.find(property_name);
      if (property_iter == element_iter->second.end()) {
        continue;
      }

      size_t original_index = property_iter->second.index();
      size_t desired_index = property_callback.index();

      if ((original_index & 0x1u) != (desired_index & 0x1u)) {
        return kProhibitedConversions[original_index][desired_index];
      }

      if (((original_index >> 1u) < 6 && (desired_index >> 1u) >= 6) ||
          ((original_index >> 1u) >= 6 && (desired_index >> 1u) < 6)) {
        return kProhibitedConversions[original_index][desired_index];
      }

      property_iter->second = std::move(property_callback);
    }
  }

  std::vector<std::vector<PropertyParser>> parsers;
  for (const PlyHeader::Element& element : header->elements) {
    parsers.emplace_back();
    for (const PlyHeader::Property& property : element.properties) {
      const PropertyCallback& callback = actual_callbacks.find(element.name)
                                             ->second.find(property.name)
                                             ->second;
      parsers.back().emplace_back(
          header->format, property.list_type, property.data_type,
          static_cast<PlyHeader::Property::Type>(callback.index() >> 1u),
          MakeHandler(callback),
          [&, this](const std::string& element_name,
                    const std::string& property_name,
                    std::error_code original_error) -> std::error_code {
            ConversionFailureReason reason{};
            switch (static_cast<ErrorCode>(original_error.value())) {
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UCHAR:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_CHAR_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_UCHAR_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_SHORT_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_USHORT_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_INT_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UCHAR:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_USHORT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_UINT:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_INT_FROM_UINT:
                reason = ConversionFailureReason::INTEGER_OVERFLOW;
                break;
              case ErrorCode::OVERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE:
              case ErrorCode::OVERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE:
                reason = ConversionFailureReason::FLOAT_OVERFLOW;
                break;
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_CHAR_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_SHORT_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_CHAR_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_SHORT_FROM_INT:
                reason = ConversionFailureReason::SIGNED_INTEGER_UNDERFLOW;
                break;
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UCHAR_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_USHORT_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_UINT_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UCHAR_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_USHORT_FROM_INT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_CHAR:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_SHORT:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_UINT_FROM_INT:
                reason = ConversionFailureReason::UNSIGNED_INTEGER_UNDERFLOW;
                break;
              case ErrorCode::UNDERFLOWED_PROPERTY_VALUE_FLOAT_FROM_DOUBLE:
              case ErrorCode::UNDERFLOWED_PROPERTY_LIST_VALUE_FLOAT_FROM_DOUBLE:
                reason = ConversionFailureReason::FLOAT_UNDERFLOW;
                break;
              default:
                break;
            }

            if (std::error_code error =
                    OnConversionFailure(element_name, property_name, reason);
                error) {
              return error;
            }

            return original_error;
          },
          element.name, property.name);
    }
  }

  static constexpr ErrorCode value_eof_error_code[8] = {
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_CHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UCHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_SHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_USHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_INT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_UINT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_FLOAT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_VALUE_DOUBLE,
  };

  static constexpr ErrorCode list_eof_error_code[6] = {
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_CHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UCHAR,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_SHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_USHORT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_INT,
      ErrorCode::UNEXPECTED_EOF_PROPERTY_LIST_SIZE_UINT,
  };

  Context context;
  context.line_ending = header->line_ending;
  for (size_t element_index = 0; element_index < header->elements.size();
       element_index++) {
    const PlyHeader::Element& element = header->elements[element_index];
    for (size_t instance = 0; instance < element.num_in_file; instance++) {
      if (header->format == PlyHeader::Format::ASCII) {
        ErrorCode eof_error = ErrorCode::UNEXPECTED_EOF_NO_PROPERTIES;
        if (!element.properties.empty()) {
          const PlyHeader::Property& property = element.properties.front();
          if (property.list_type) {
            eof_error =
                list_eof_error_code[static_cast<size_t>(*property.list_type)];
          } else {
            eof_error =
                value_eof_error_code[static_cast<size_t>(property.data_type)];
          }
        }

        if (std::error_code error = ReadNextLine(stream, context, eof_error);
            error) {
          return error;
        }
      }

      for (size_t property_index = 0;
           property_index < header->elements[element_index].properties.size();
           property_index++) {
        if (std::error_code error =
                parsers[element_index][property_index].Parse(stream, context);
            error) {
          return error;
        }
      }

      if (header->format == PlyHeader::Format::ASCII) {
        std::error_code error = ReadNextToken(
            context, false, ErrorCode::ASCII_UNUSED_TOKEN,
            ErrorCode::ASCII_EMPTY_TOKEN, ErrorCode::ASCII_EMPTY_TOKEN);
        if (!error) {
          return ErrorCode::ASCII_UNUSED_TOKEN;
        } else if (error != ErrorCode::ASCII_EMPTY_TOKEN) {
          return error;
        }
      }
    }
  }

  return std::error_code();
}

// Static assertions to ensure float types are properly sized
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8);
static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4);

// Static assertions to ensure system does not use mixed endianness
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

}  // namespace plyodine
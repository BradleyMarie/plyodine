#include "plyodine/error_code.h"

#include <system_error>

#include "plyodine/error_codes.h"

namespace plyodine {
namespace internal {
namespace {

class ErrorCategory final : public std::error_category {
  const char* name() const noexcept override;
  std::string message(int condition) const override;
};

const char* ErrorCategory::name() const noexcept { return "plyodine"; }

std::string ErrorCategory::message(int condition) const {
  ErrorCode error_code{condition};
  switch (error_code) {
    case ErrorCode::BAD_STREAM:
      return "Bad stream passed";
    case ErrorCode::HEADER_MISSING_MAGIC_STRING:
      return "The first line of the input must exactly contain the magic "
             "string";
    case ErrorCode::HEADER_CONTAINS_MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::HEADER_CONTAINS_INVALID_CHARACTER:
      return "The input contained an invalid character";
    case ErrorCode::HEADER_LINE_STARTS_WITH_WHITESPACE:
      return "ASCII lines may not begin with a space";
    case ErrorCode::HEADER_LINE_ENDS_WITH_WHITESPACE:
      return "Non-comment ASCII lines may not contain trailing spaces";
    case ErrorCode::HEADER_LINE_CONTAINS_EXTRA_WHITESPACE:
      return "Non-comment ASCII lines may only contain a single space between "
             "tokens";
    case ErrorCode::HEADER_MISSING_FORMAT_SPECIFIER:
      return "The second line of the input must contain the format specifier";
    case ErrorCode::HEADER_SPECIFIED_INVALID_FORMAT:
      return "Format must be one of ascii, binary_big_endian, or "
             "binary_little_endian";
    case ErrorCode::HEADER_SPECIFIED_UNSUPPORTED_VERSION:
      return "Only PLY version 1.0 supported";
    case ErrorCode::HEADER_FORMAT_SPECIFIER_TOO_LONG:
      return "The format specifier contained too many parameters";
    case ErrorCode::HEADER_NAKED_PROPERTY:
      return "A property could not be associated with an element";
    case ErrorCode::HEADER_PROPERTY_SPECIFIER_TOO_SHORT:
      return "A property specifier contained too few parameters";
    case ErrorCode::HEADER_PROPERTY_SPECIFIED_INVALID_TYPE:
      return "A property is of an invalid type";
    case ErrorCode::HEADER_PROPERTY_SPECIFIED_LIST_TYPE_FLOAT:
      return "A property list cannot have float as its list type";
    case ErrorCode::HEADER_PROPERTY_SPECIFIED_LIST_TYPE_DOUBLE:
      return "A property list cannot have double as its list type";
    case ErrorCode::HEADER_PROPERTY_SPECIFIED_DUPLICATE_NAME:
      return "An element contains two properties with the same name";
    case ErrorCode::HEADER_PROPERTY_SPECIFIER_TOO_LONG:
      return "Too many prameters to property";
    case ErrorCode::HEADER_ELEMENT_SPECIFIER_TOO_SHORT:
      return "Too few prameters to element";
    case ErrorCode::HEADER_ELEMENT_SPECIFIED_DUPLICATE_NAME:
      return "Two elements have the same name";
    case ErrorCode::HEADER_ELEMENT_COUNT_OUT_OF_RANGE:
      return "Out of range element count";
    case ErrorCode::HEADER_ELEMENT_COUNT_PARSING_FAILED:
      return "Failed to parse element count";
    case ErrorCode::HEADER_ELEMENT_SPECIFIER_TOO_LONG:
      return "Too many prameters to element";
    case ErrorCode::HEADER_END_INVALID:
      return "The last line of the header may only contain the end_header "
             "keyword";
    case ErrorCode::HEADER_UNRECOGNIZED_KEYWORD:
      return "The input contained an invalid header";
    case ErrorCode::READER_UNEXPECTED_EOF:
      return "Unexpected EOF";
    case ErrorCode::READER_CONTAINS_MISMATCHED_LINE_ENDINGS:
      return "The input contained mismatched line endings";
    case ErrorCode::READER_CONTAINS_INVALID_CHARACTER:
      return "The input contained an invalid character";
    case ErrorCode::READER_NEGATIVE_LIST_SIZE:
      return "The input contained a property list with a negative size";
    case ErrorCode::READER_ELEMENT_TOO_FEW_TOKENS:
      return "The input contained an element with too few tokens";
    case ErrorCode::READER_ELEMENT_CONTAINS_EXTRA_WHITESPACE:
      return "Non-comment ASCII lines may only contain a single space between "
             "tokens";
    case ErrorCode::READER_ELEMENT_CONTAINS_EXTRA_TOKENS:
      return "The input contained an element with unused tokens";
    case ErrorCode::READER_ELEMENT_LIST_SIZE_OUT_OF_RANGE:
      return "The input contained a property list size that was out of range";
    case ErrorCode::READER_ELEMENT_PROPERTY_OUT_OF_RANGE:
      return "The input contained a property entry that was out of range";
    case ErrorCode::READER_ELEMENT_LIST_SIZE_PARSING_FAILED:
      return "The input contained a property list size that failed to parse";
    case ErrorCode::READER_ELEMENT_PROPERTY_PARSING_FAILED:
      return "The input contained a property entry that failed to parse";
    case ErrorCode::WRITER_WRITE_ERROR:
      return "Write failure";
    case ErrorCode::WRITER_COMMENT_CONTAINS_NEWLINE:
      return "A comment may not contain line feed or carriage return";
    case ErrorCode::WRITER_OBJ_INFO_CONTAINS_NEWLINE:
      return "An obj_info may not contain line feed or carriage return";
    case ErrorCode::WRITER_EMPTY_NAME_SPECIFIED:
      return "Names of properties and elements may not be empty";
    case ErrorCode::WRITER_NAME_CONTAINED_INVALID_CHARACTERS:
      return "Names of properties and elements may only contain graphic "
             "characters";
    case ErrorCode::WRITER_LIST_INDEX_TOO_SMALL:
      return "The list was too big to be represented with the selected size "
             "type";
    case ErrorCode::WRITER_ASCII_FLOAT_NOT_FINITE:
      return "Only finite floating point values may be serialized to an ASCII "
             "output";
  };

  return "Unknown Error";
}

}  // namespace

std::error_code MakeErrorCode(ErrorCode code) {
  static const ErrorCategory error_category;
  return std::error_code(static_cast<int>(code), error_category);
}

}  // namespace internal
}  // namespace plyodine
#ifndef _PLYODINE_ERROR_CODE_
#define _PLYODINE_ERROR_CODE_

#include <expected>
#include <system_error>

#include "plyodine/error_codes.h"

namespace plyodine {
namespace internal {

std::error_code MakeErrorCode(ErrorCode code);

std::unexpected<std::error_code> MakeUnexpected(ErrorCode code) {
  return std::unexpected(MakeErrorCode(code));
}

}  // namespace internal
}  // namespace plyodine

#endif  // _PLYODINE_ERROR_CODE_
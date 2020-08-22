#include "tria/math/utils.hpp"

#if defined(TRIA_WIN32)
#include <intrin.h>

#if defined(_MSC_VER)
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#endif

#endif

namespace tria::math {

auto popCount(uint32_t mask) noexcept -> unsigned int {
#if defined(TRIA_WIN32)
  return __popcnt(mask);
#endif
  return __builtin_popcount(mask);
}

auto countTrailingZeroes(uint32_t mask) noexcept -> unsigned int {
  if (mask == 0U) {
    return 32U;
  }
#if defined(TRIA_WIN32)
  unsigned long result;
  _BitScanForward(&result, mask);
  return static_cast<unsigned int>(result);
#endif
  return __builtin_ctz(mask);
}

auto countLeadingZeroes(uint32_t mask) noexcept -> unsigned int {
  if (mask == 0U) {
    return 32U;
  }
#if defined(TRIA_WIN32)
  unsigned long result;
  _BitScanReverse(&result, mask);
  return static_cast<unsigned int>(31U - result);
#endif
  return __builtin_clz(mask);
}

} // namespace tria::math

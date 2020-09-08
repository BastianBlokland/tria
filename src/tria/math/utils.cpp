#include "tria/math/utils.hpp"
#include <array>

#if defined(TRIA_WIN32)
#include <intrin.h>
#endif

#if defined(_MSC_VER)

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

#else // !defined(_MSC_VER)

#include <cpuid.h>
#include <immintrin.h>

#endif

namespace tria::math {

namespace {

[[nodiscard]] auto asUint(float x) noexcept -> uint32_t { return *reinterpret_cast<uint32_t*>(&x); }

[[nodiscard]] auto asFloat(uint32_t x) noexcept -> float { return *reinterpret_cast<float*>(&x); }

void getCpuId(int functionId, std::array<int, 4>& result) {
#if defined(_MSC_VER)
  __cpuid(result.data(), functionId);
#else
  __cpuid(functionId, result[0], result[1], result[2], result[3]);
#endif
}

/* Check if the cpu supports the f16c (16 bit float conversions) instructions.
 * More info on cpu feature flags:
 * https://en.wikipedia.org/wiki/CPUID#EAX=1:_Processor_Info_and_Feature_Bits
 */
[[nodiscard]] auto checkSupportF16c() {
  std::array<int, 4> result;
  getCpuId(1, result);
  return result[2] & 0x20000000;
}

} // namespace

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

namespace {

auto floatToHalfF16c(float val) noexcept -> uint16_t {
  // Intel intrinsic for converting float to half.
  // https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_cvtss_sh
#if defined(_MSC_VER)
  // MSVC doesn't define the single value '_cvtss_sh'.
  return _mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(val), _MM_FROUND_TO_NEAREST_INT));
#else
  return _cvtss_sh(val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#endif
}

auto floatToHalfSoft(float val) noexcept -> uint16_t {
  /* IEEE-754 16-bit floating-point format (without infinity):
   * 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
   *
   * Source: Awnser of user 'ProjectPhysX' on the following StackOverflow question:
   * https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
   */

  const uint32_t b =
      asUint(val) + 0x00001000; // Round-to-nearest-even: add last bit after truncated mantissa
  const uint32_t e = (b & 0x7F800000) >> 23; // exponent
  const uint32_t m = b & 0x007FFFFF; // Mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000
                                     // = decimal indicator flag - initial rounding
  return (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) |
      ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) |
      (e > 143) * 0x7FFF; // Sign : normalized : denormalized : saturate
}

} // namespace

auto floatToHalf(float val) noexcept -> uint16_t {
  static auto impl = checkSupportF16c() ? &floatToHalfF16c : &floatToHalfSoft;
  return impl(val);
}

namespace {

auto halfToFloatF16c(uint16_t val) noexcept -> float {
  // Intel intrinsic for converting half to float.
  // https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_cvtsh_ss
#if defined(_MSC_VER)
  // MSVC doesn't define the single value '_cvtsh_ss'.
  return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(val)));
#else
  return _cvtsh_ss(val);
#endif
}

auto halfToFloatSoft(uint16_t val) noexcept -> float {
  /* IEEE-754 16-bit floating-point format (without infinity):
   * 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
   *
   * Source: Awnser of user 'ProjectPhysX' on the following StackOverflow question:
   * https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
   */

  const uint32_t e = (val & 0x7C00) >> 10; // Exponent
  const uint32_t m = (val & 0x03FF) << 13; // Mantissa

  // Evil log2 bit hack to count leading zeros in denormalized format:
  const uint32_t v = asUint(static_cast<float>(m)) >> 23;

  return asFloat(
      (val & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) |
      ((e == 0) & (m != 0)) *
          ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // Sign : normalized : denormalized
}

} // namespace

auto halfToFloat(uint16_t val) noexcept -> float {
  static auto impl = checkSupportF16c() ? &halfToFloatF16c : &halfToFloatSoft;
  return impl(val);
}

} // namespace tria::math

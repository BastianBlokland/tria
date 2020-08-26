#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

namespace tria::math {

/* Constant for π.
 */
template <typename T>
constexpr auto pi = T{3.141592653589793238463};

/* Constant to convert from radians to degrees.
 */
template <typename T>
constexpr auto radToDeg = T{180.0} / pi<T>;

/* Constant to convert from degrees to radians.
 */
template <typename T>
constexpr auto degToRad = pi<T> / T{180.0};

/* Check if two floating point numbers are approximately equal.
 * Note: Should not be used to compare to zero, use 'approxZero' instead.
 */
template <typename T>
[[nodiscard]] constexpr auto
approx(T x, T y, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  /* If the difference is less then the epsilon of 1 then return true. If not we do another check
   * but this time with the epislon scaled to the biggest of the two values.
   */
  auto diff = std::fabs(x - y);
  return diff <= maxDelta || diff < std::fmax(std::fabs(x), std::fabs(y)) * maxDelta;
}

/* Check if the given floating point number is approximately zero.
 */
template <typename T>
[[nodiscard]] constexpr auto
approxZero(T x, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  return std::fabs(x) <= maxDelta;
}

/* Return the linearly interpolated value from x to y at time t.
 * Note: Does not clamp t (so can extrapolate too).
 */
template <typename T>
[[nodiscard]] constexpr auto lerp(T x, T y, float t) noexcept -> T {
  return x + (y - x) * t;
}

/* Opposite of lerp, returns at what t the value lies in respect to x and y.
 * Note: does not clamp the value (so can return less then 0 or more then 1).
 */
template <typename T>
[[nodiscard]] constexpr auto unlerp(T x, T y, T value) noexcept -> float {
  return (x == y) ? T{} : (value - x) / (y - x);
}

/* Returns how many bits are set in a mask.
 */
[[nodiscard]] auto popCount(uint32_t mask) noexcept -> unsigned int;

/* Returns the amount of trailing zeroes in a mask.
 * Note: returns 32 for a mask with 0 set bits.
 */
[[nodiscard]] auto countTrailingZeroes(uint32_t mask) noexcept -> unsigned int;

/* Returns the amount of leading zeroes in a mask.
 * Note: returns 32 for a mask with 0 set bits.
 */
[[nodiscard]] auto countLeadingZeroes(uint32_t mask) noexcept -> unsigned int;

/* Integer version of log base 2.
 * Undefined for val == 0.
 */
[[nodiscard]] inline auto log2i(uint32_t val) noexcept {
  assert(val != 0U);
  return 31U - countLeadingZeroes(val);
}

/* Check if the given value is a power of two.
 * Undefined for val == 0.
 */
[[nodiscard]] constexpr auto isPow2(uint32_t val) noexcept {
  assert(val != 0U);
  // Ref: https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2.
  return (val & (val - 1U)) == 0;
}

/* Return the next power of two greater or equal to val.
 * Undefined for val == 0 and val > 2147483648
 */
[[nodiscard]] inline auto nextPow2(uint32_t val) noexcept {
  assert(val != 0U);
  assert(val <= 2147483648U);
  return 1U << (32U - countLeadingZeroes(val - 1U));
}

/* Create a (non cryptographic) hash of the input data.
 */
[[nodiscard]] constexpr auto hash(const void* data, size_t dataSize) noexcept -> uint32_t {
  /* Fowler–Noll–Vo hash function.
   * Ref: http://www.isthe.com/chongo/tech/comp/fnv/index.html
   * Ref: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
   *
   * FNV-1a.
   * 32-bit
   * prime: 2^24 + 2^8 + 0x93 = 16777619
   * offset: 2166136261
   */

  const auto* dataPtr = static_cast<const uint8_t*>(data);
  const auto* dataEnd = dataPtr + dataSize;

  constexpr auto prime = 16777619U;
  uint32_t hash        = 2166136261U;

  for (; dataPtr != dataEnd; ++dataPtr) {
    hash ^= *dataPtr;
    hash *= prime;
  }

  // Finalize the hash (aka 'mixing').
  hash += hash << 13U;
  hash ^= hash >> 7U;
  hash += hash << 3U;
  hash ^= hash >> 17U;
  hash += hash << 5U;
  return hash;
}

} // namespace tria::math

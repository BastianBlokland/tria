#pragma once
#include <cmath>

namespace tria::math {

/* Check if two floating point numbers are approximately equal.
 * Note: Should not be used to compare to zero, use 'approxZero' instead.
 */
template <typename T>
constexpr auto approx(T x, T y, T maxDelta = std::numeric_limits<T>::epsilon()) {
  /* If the difference is less then the epsilon of 1 then return true. If not we do another check
   * but this time with the epislon scaled to the biggest of the two values.
   */
  auto diff = std::fabs(x - y);
  return diff <= maxDelta || diff < std::fmax(std::fabs(x), std::fabs(y)) * maxDelta;
}

/* Check if the given floating point number is approximately zero.
 */
template <typename T>
constexpr auto approxZero(T x, T maxDelta = std::numeric_limits<T>::epsilon()) {
  return std::fabs(x) <= maxDelta;
}

} // namespace tria::math

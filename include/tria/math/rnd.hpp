#pragma once
#include "tria/math/utils.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <utility>

namespace tria::math {

/* Rng implementation using the xorwow algorithm.
 * Uses a shift-register implementation, usefull for usecases where distribution and repetition
 * period are not critically important. Do not use this for anything security related.
 */
class RngXorWow final {
public:
  /* Initialize xorwow with a seed based on the system clock.
   * Note: using this constructor makes the rng non-deterministic.
   */
  RngXorWow() noexcept;

  /* Initialize xorwow from an explicit seed.
   */
  explicit RngXorWow(uint64_t seed) noexcept;

  /* Get the next value in the sequence.
   * Returns a float between 0.0 (inclusive) and 1.0 (exclusive).
   */
  [[nodiscard]] auto next() noexcept -> float;

private:
  std::array<uint32_t, 5> m_state;
};

/* Global default random number generator.
 * Uses a random sequence per thread intialized with a seed based on the system-clock.
 */
thread_local extern RngXorWow g_rng;

/* Get the next value in the random sequence.
 * Returns a float between 0.0 (inclusive) and 1.0 (exclusive) with a uniform distribution.
 */
template <typename Rng>
[[nodiscard]] auto rndSample(Rng& rng) noexcept -> float {
  return rng.next();
}

/* Get the next value in the random sequence.
 * Returns a value between min (inclusive) and max (exclusive) with a uniform distribution.
 */
template <typename Rng, typename T>
[[nodiscard]] auto rndSample(Rng& rng, T min, T max) noexcept -> T {
  const auto range = max - min;
  return min + static_cast<T>(range * rndSample(rng));
}

/* Get the next two values with a gaussian (normal) distribution.
 */
template <typename Rng>
[[nodiscard]] auto rndSampleGauss(Rng& rng) noexcept -> std::pair<float, float> {
  float a, b;
  do {
    a = rng.next();
    b = rng.next();
    // Guard against a value very close to zero as we will feed it into std::log.
  } while (a <= std::numeric_limits<float>::epsilon());
  // BoxMuller transform.
  // Source: https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
  return {std::sqrt(-2.0f * std::log(a)) * std::cos(pi<float> * 2.0f * b),
          std::sqrt(-2.0f * std::log(a)) * std::sin(pi<float> * 2.0f * b)};
}

} // namespace tria::math

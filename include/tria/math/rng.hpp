#pragma once
#include <array>
#include <cstdint>
#include <limits>

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
  RngXorWow();

  /* Initialize xorwow from an explicit seed.
   */
  explicit RngXorWow(uint64_t seed);

  /* Get the next value in the sequence.
   * Returns a float between 0.0 (inclusive) and 1.0 (exclusive).
   */
  [[nodiscard]] auto next() noexcept -> float;

private:
  std::array<uint32_t, 5> m_state;
};

/* Get the next value in the rng sequence.
 * Returns a float between 0.0 (inclusive) and 1.0 (exclusive).
 * Uses a random sequence per thread intialized with a seed based on the system-clock.
 * Return values are non-deterministic.
 */
[[nodiscard]] auto rngNext() -> float;

/* Get the next value in the rng sequence.
 * Returns a float between 0.0 (inclusive) and 1.0 (exclusive).
 */
template <typename Rng>
[[nodiscard]] auto rngNext(Rng& rng) -> float {
  return rng.next();
}

/* Get the next value in the rng sequence.
 * Returns a value between min (inclusive) and max (exclusive).
 * Uses a random sequence per thread intialized with a seed based on the system-clock.
 * Return values are non-deterministic.
 */
template <typename T>
[[nodiscard]] auto rngNext(T min, T max) -> T {
  const auto range = max - min;
  return min + static_cast<T>(range * rngNext());
}

/* Get the next value in the rng sequence.
 * Returns a value between min (inclusive) and max (exclusive).
 */
template <typename Rng, typename T>
[[nodiscard]] auto rngNext(Rng& rng, T min, T max) -> T {
  const auto range = max - min;
  return min + static_cast<T>(range * rngNext(rng));
}

} // namespace tria::math

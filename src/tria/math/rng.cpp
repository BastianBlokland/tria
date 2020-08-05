#include "tria/math/rng.hpp"
#include <cassert>
#include <ctime>
#include <limits>

namespace tria::math {

namespace {

/* Implementation of the 'splitmix' algorithm.
 * Source: https://en.wikipedia.org/wiki/Xorshift#xorwow
 */
[[nodiscard]] auto splitmix64(uint64_t& state) -> uint64_t {
  auto result = state;
  state       = result + 0x9E3779B97f4A7C15;
  result      = (result ^ (result >> 30U)) * 0xBF58476D1CE4E5B9;
  result      = (result ^ (result >> 27U)) * 0x94D049BB133111EB;
  return result ^ (result >> 31U);
}

/* Implementation of the 'xorwow' algorithm.
 * Note: First four elements of the state array should be initialized to non-zero.
 * Source: https://en.wikipedia.org/wiki/Xorshift#xorwow
 */
[[nodiscard]] auto xorwow(std::array<uint32_t, 5>& state) -> uint32_t {
  assert(state[0] != 0U);
  assert(state[1] != 0U);
  assert(state[2] != 0U);
  assert(state[3] != 0U);

  auto& counter = state[4];
  auto t        = state[3];
  const auto s  = state[0];
  state[3]      = state[2];
  state[2]      = state[1];
  state[1]      = s;

  t ^= t >> 2U;
  t ^= t << 1U;
  t ^= s ^ (s << 4U);
  state[0] = t;

  counter += 362437U;
  return t + counter;
}

} // namespace

RngXorWow::RngXorWow() : RngXorWow{static_cast<uint64_t>(std::clock())} {}

RngXorWow::RngXorWow(uint64_t seed) {
  // 0 is a seed we cannot recover from.
  // TODO(bastian): Should just document the precondition that you are not allowed to use seed 0?
  if (seed == 0U) {
    seed = 1U;
  }

  // Initialize the state for xorwow using the splitmix algorithm.
  auto val1  = splitmix64(seed);
  auto val2  = splitmix64(seed);
  m_state[0] = static_cast<uint32_t>(val1);
  m_state[1] = static_cast<uint32_t>(val1 >> 32U);
  m_state[2] = static_cast<uint32_t>(val2);
  m_state[3] = static_cast<uint32_t>(val2 >> 32U);
  m_state[4] = 0U;
}

auto RngXorWow::next() noexcept -> float {
  // Note: +1 as we never want to return 1.0.
  constexpr auto max = static_cast<float>(std::numeric_limits<uint32_t>::max()) + 1.0f;
  return xorwow(m_state) / max;
}

auto rngNext() -> float {
  thread_local static auto rng = RngXorWow{};
  return rng.next();
}

} // namespace tria::math

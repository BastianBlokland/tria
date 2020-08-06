#include "catch2/catch.hpp"
#include "tria/math/rnd.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Random", "[math]") {

  SECTION("RngXorWow returns values between 0.0 (inclusive) and 1.0 (exclusive)") {
    auto rng = RngXorWow{42};
    for (auto i = 0U; i != 1'000; ++i) {
      const auto next = rng.next();
      CHECK(next >= .0f);
      CHECK(next < 1.0f);
    }
  }

  SECTION("rndSample never returns max") {
    auto rng = RngXorWow{42};
    for (auto i = 0U; i != 1'000; ++i) {
      CHECK(rndSample(rng, 42, 43) == 42);
    }
  }

  SECTION("Empty RngXorWow constructor uses seed based on clock") {
    auto rng = RngXorWow{};
    CHECK(rng.next() >= 0);
  }

  SECTION("Initializing RngXorWow with 0 seed is allowed") {
    auto rng = RngXorWow{0};
    CHECK(rng.next() >= 0);
  }
}

} // namespace tria::math::tests

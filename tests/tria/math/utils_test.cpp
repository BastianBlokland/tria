#include "catch2/catch.hpp"
#include "tria/math/utils.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Utils", "[math]") {

  SECTION("Approx checks if values are approximately equal") {
    CHECK(approx(1.0f, 1.0f));
    CHECK(!approx(1.0f, 1.001f));
    CHECK(approx(1.0f, 1.0000001f));
  }

  SECTION("ApproxZero check if a value is approximately zero") {
    CHECK(approxZero(0.0f));
    CHECK(!approxZero(0.001f));
    CHECK(approxZero(0.0000001f));
  }
}

} // namespace tria::math::tests

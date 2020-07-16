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

  SECTION("Lerp at t = 0 returns x") {
    CHECK(lerp(42, 1337, 0.0f) == 42);
    CHECK(lerp(-42, 1337, 0.0f) == -42);
    CHECK(lerp(1337, 42, 0.0f) == 1337);
  }

  SECTION("Lerp at t = 1 returns y") {
    CHECK(lerp(42, 1337, 1.0f) == 1337);
    CHECK(lerp(42, -1337, 1.0f) == -1337);
    CHECK(lerp(1337, 42, 1.0f) == 42);
  }

  SECTION("Lerp at t = 2 returns y + (y - x)") {
    CHECK(lerp(10, 20, 2.0f) == 30);
    CHECK(lerp(-10, -20, 2.0f) == -30);
  }

  SECTION("Unlerp at value = x returns 0") {
    CHECK(approx(unlerp(10, 20, 10), 0.0f));
    CHECK(approx(unlerp(-10, -20, -10), 0.0f));
  }

  SECTION("Unlerp at value = y returns 1") {
    CHECK(approx(unlerp(10, 20, 20), 1.0f));
    CHECK(approx(unlerp(-10, -20, -20), 1.0f));
  }

  SECTION("Unlerp at value = y + (y - x) returns 2") {
    CHECK(approx(unlerp(10, 20, 30), 2.0f));
    CHECK(approx(unlerp(-10, -20, -30), 2.0f));
  }

  SECTION("Unlerp can undo lerp") {
    const auto x   = 42.1337f;
    const auto y   = -47.3f;
    const auto t   = -3.153f;
    const auto val = lerp(x, y, t);
    CHECK(approx(unlerp(x, y, val), t));
  }
}

} // namespace tria::math::tests

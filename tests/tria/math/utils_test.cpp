#include "catch2/catch.hpp"
#include "tria/math/utils.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Utils", "[math]") {

  SECTION("45 degrees is quater pi in radians") {
    CHECK(approx(45.0f * degToRad<float>, pi<float> / 4.0f));
  }

  SECTION("pi in radians is 180 degrees") { CHECK(approx(pi<float> * radToDeg<float>, 180.0f)); }

  SECTION("radToDeg and degToRad roundtrip") {
    CHECK(approx(45.0f * degToRad<float> * radToDeg<float>, 45.0f));
  }

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

  SECTION("Approx on integers uses whole numbers") {
    CHECK(approx(1, 1));
    CHECK(!approx(1, 2));
    CHECK(approxZero(0));
    CHECK(!approxZero(1));
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

  SECTION("Mask population count") {
    CHECK(popCount(0b0000'0000'0000'0000'0000'0000'0000'0000) == 0);
    CHECK(popCount(0b0000'0000'0000'0000'0000'0000'0000'0001) == 1);
    CHECK(popCount(0b1000'0000'0000'0000'0000'0000'0000'0000) == 1);
    CHECK(popCount(0b0001'0000'0000'0000'0000'0000'0000'0000) == 1);
    CHECK(popCount(0b0000'0010'0000'0000'0000'0000'0100'0000) == 2);
    CHECK(popCount(0b1000'0010'0000'0010'0010'0100'0100'0101) == 8);
    CHECK(popCount(0xFFFF'FFFF) == 32);
  }

  SECTION("Mask count trailing zeroes") {
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'1100'0111) == 0);
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'1100'0110) == 1);
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'1100'0100) == 2);
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'1100'0000) == 6);
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'1000'0000) == 7);
    CHECK(countTrailingZeroes(0b0100'0100'0100'0100'0100'0100'0000'0000) == 10);
    CHECK(countTrailingZeroes(0b1000'0000'0000'0000'0000'0000'0000'0000) == 31);
    CHECK(countTrailingZeroes(0b0000'0000'0000'0000'0000'0000'0000'0000) == 32);
  }

  SECTION("Mask count leading zeroes") {
    CHECK(countLeadingZeroes(0b1100'0100'0100'0100'0100'0100'1100'0100) == 0);
    CHECK(countLeadingZeroes(0b0100'0100'0100'0100'0100'0100'1100'0101) == 1);
    CHECK(countLeadingZeroes(0b0011'1100'0100'0100'0100'0100'1100'0100) == 2);
    CHECK(countLeadingZeroes(0b0000'0010'0110'0100'0100'0100'1100'1100) == 6);
    CHECK(countLeadingZeroes(0b0000'0001'1100'0100'0100'0100'1000'0011) == 7);
    CHECK(countLeadingZeroes(0b0000'0000'0011'0100'0100'0100'0001'0000) == 10);
    CHECK(countLeadingZeroes(0b0000'0000'0000'0000'0000'0000'0000'0001) == 31);
    CHECK(countLeadingZeroes(0b0000'0000'0000'0000'0000'0000'0000'0000) == 32);
  }

  SECTION("Integer log2") {
    CHECK(log2i(1) == 0);
    CHECK(log2i(2) == 1);
    CHECK(log2i(3) == 1);
    CHECK(log2i(4) == 2);
    CHECK(log2i(5) == 2);
    CHECK(log2i(6) == 2);
    CHECK(log2i(8) == 3);
    CHECK(log2i(16) == 4);
    CHECK(log2i(32) == 5);
    CHECK(log2i(33) == 5);
    CHECK(log2i(63) == 5);
    CHECK(log2i(64) == 6);
  }
}

} // namespace tria::math::tests

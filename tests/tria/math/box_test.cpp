#include "catch2/catch.hpp"
#include "tria/math/box.hpp"
#include "tria/math/box_io.hpp"
#include "tria/math/utils.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Axis aligned box", "[math]") {

  SECTION("Box's are fixed size") {
    const auto b1 = Box3f{};
    CHECK(sizeof(b1) == sizeof(float) * 6);

    const auto b2 = Box<double, 3>{};
    CHECK(sizeof(b2) == sizeof(double) * 6);
  }

  SECTION("Box's can be checked for equality") {
    CHECK(invertedBox3f() == invertedBox3f());
    CHECK(!(Box3f{} == invertedBox3f()));
  }

  SECTION("Box's can be checked for inequality") {
    CHECK(Box3f{} != invertedBox3f());
    CHECK(!(invertedBox3f() != invertedBox3f()));
  }

  SECTION("A box is formed from the min and max points") {
    const auto min = Vec3f{-1.f, -1.f, -1.f};
    const auto max = Vec3f{+1.f, +1.f, +1.f};
    const auto b   = Box3f{min, max};
    CHECK(approxZero(b.getCenter()));
    CHECK(approx(b.getSize(), {2.f, 2.f, 2.f}));
  }

  SECTION("Inverted box is infinitely small") {
    const auto b = invertedBox3f();
    CHECK(b.getSize().x() < -9999999.f);
    CHECK(b.getSize().y() < -9999999.f);
    CHECK(b.getSize().z() < -9999999.f);
  }

  SECTION("Encapsulating a single point results in a zero volume box around that point") {
    const auto p = Vec3f{.1337f, -42.f, 123.f};
    auto b       = invertedBox3f();
    b.encapsulate(p);
    CHECK(approx(b.getCenter(), p));
    CHECK(approxZero(b.getSize()));
  }

  SECTION("Encapsulating multiple results in a box that tightly fits around the points") {
    const auto p1 = Vec3f{.1337f, 0.0f, -1.f};
    const auto p2 = Vec3f{.1337f, 0.0f, +2.f};
    const auto p3 = Vec3f{.1337f, 0.0f, +1.f};
    auto b        = invertedBox3f();
    b.encapsulate(p1);
    b.encapsulate(p2);
    b.encapsulate(p3);

    CHECK(approxZero(b.getSize().x()));
    CHECK(approxZero(b.getSize().y()));
    CHECK(approx(b.getSize().z(), 3.f));
  }
}

} // namespace tria::math::tests

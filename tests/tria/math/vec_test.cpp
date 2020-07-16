#include "catch2/catch.hpp"
#include "tria/math/vec.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Vec", "[math]") {

  SECTION("Vectors are constructed from their components") {
    const auto vf = Vec3f{1.0, 2.0, 3.0};
    CHECK(vf.x() == 1.0);
    CHECK(vf.y() == 2.0);
    CHECK(vf.z() == 3.0);

    const auto vi = Vec3i{1, 2, 3};
    CHECK(vi.x() == 1);
    CHECK(vi.y() == 2);
    CHECK(vi.z() == 3);
  }

  SECTION("Vectors can be constructed from an initializer list") {
    const Vec3f v1 = {1.0, 2.0, 3.0};
    CHECK(v1.x() == 1.0);
    CHECK(v1.y() == 2.0);
    CHECK(v1.z() == 3.0);

    const Vec3i v2 = {1, 2, 3};
    CHECK(v2.x() == 1);
    CHECK(v2.y() == 2);
    CHECK(v2.z() == 3);

    const Vec3i v3 = {};
    CHECK(v3.x() == 0);
    CHECK(v3.y() == 0);
    CHECK(v3.z() == 0);

    const Vec3i v4 = {1};
    CHECK(v4.x() == 1);
    CHECK(v4.y() == 0);
    CHECK(v4.z() == 0);

    const Vec3i v5 = {1, 2};
    CHECK(v5.x() == 1);
    CHECK(v5.y() == 2);
    CHECK(v5.z() == 0);
  }

  SECTION("Vectors can be decomposed using structured bindings") {
    auto [x1, y1, z1] = Vec3i{1, 2, 3};
    CHECK(x1 == 1);
    CHECK(y1 == 2);
    CHECK(z1 == 3);

    const auto vec2   = Vec3i{1, 2, 3};
    auto [x2, y2, z2] = vec2;
    CHECK(x2 == 1);
    CHECK(y2 == 2);
    CHECK(z2 == 3);

    auto vec3          = Vec3i{1, 2, 3};
    auto& [x3, y3, z3] = vec3;
    x3                 = 42;
    y3                 = 1337;
    CHECK(vec3.x() == 42);
    CHECK(vec3.y() == 1337);
    CHECK(z3 == 3);
    CHECK(vec3.z() == 3);
  }

  SECTION("Component aliases represent the same value") {
    auto vf = Vec4i{};
    CHECK(vf[0] == 0);
    CHECK(vf[1] == 0);
    CHECK(vf[2] == 0);
    CHECK(vf[3] == 0);

    vf.x() = 1;
    CHECK(vf.r() == 1);
    CHECK(vf[0] == 1);

    vf.y() = 2;
    CHECK(vf.g() == 2);
    CHECK(vf[1] == 2);

    vf.z() = 3;
    CHECK(vf.b() == 3);
    CHECK(vf[2] == 3);

    vf.w() = 4;
    CHECK(vf.a() == 4);
    CHECK(vf[3] == 4);
  }

  SECTION("Vectors can be checked for equality") {
    CHECK(!(Vec3i{1, 2, 3} == Vec3i{5, 7, 9}));
    CHECK(Vec3i{1, 2, 3} == Vec3i{1, 2, 3});
  }

  SECTION("Vectors can be checked for inequality") {
    CHECK(Vec3i{1, 2, 3} != Vec3i{5, 7, 9});
    CHECK(!(Vec3i{1, 2, 3} != Vec3i{1, 2, 3}));
  }

  SECTION("Adding vectors sums each component") {
    CHECK(Vec3i{1, 2, 3} + Vec3i{4, 5, 6} == Vec3i{5, 7, 9});
  }

  SECTION("Substracting vectors substracts each component") {
    CHECK(Vec3i{1, 2, 3} - Vec3i{4, 5, 6} == Vec3i{-3, -3, -3});
  }

  SECTION("Inverting vectors inverts each component") {
    CHECK(-Vec3i{2, 4, 6} == Vec3i{-2, -4, -6});
  }

  SECTION("Multipling vectors by a scalar scales each component") {
    CHECK(Vec3i{1, 2, 3} * 2 == Vec3i{2, 4, 6});
  }

  SECTION("Dividing vectors by a scalar divides each component") {
    CHECK(Vec3i{2, 4, 6} / 2 == Vec3i{1, 2, 3});
  }

  SECTION("Square magnitude is sum of squared components") {
    const auto vec = Vec3f{1.0f, 2.0f, 3.0f};
    CHECK(approx(vec.getSqrMag(), 14.0f));
  }

  SECTION("Magnitude is the square root of the sum of squared components") {
    const auto vec = Vec3f{0.0f, 42.0f, 0.0f};
    CHECK(approx(vec.getMag(), 42.0f));
  }

  SECTION("Normalizing a vector results in a vector with length 1") {
    const auto vec1 = Vec3f{0.0f, 42.0f, 0.0f};
    CHECK(vec1.getNorm() == Vec3f{0.0f, 1.0f, 0.0f});

    const auto vec2 = Vec3f{1337.0f, 42.0f, -42.0f};
    CHECK(approx(vec2.getNorm().getMag(), 1.0f));
  }
}

} // namespace tria::math::tests

#include "catch2/catch.hpp"
#include "tria/math/vec.hpp"
#include <unordered_set>

namespace tria::math::tests {

TEST_CASE("[math] - Vec", "[math]") {

  SECTION("Vectors are fixed size") {
    auto vf = Vec3f{};
    CHECK(sizeof(vf) == sizeof(float) * 3);

    auto vc = Vec<char, 16>{};
    CHECK(sizeof(vc) == sizeof(char) * 16);
  }

  SECTION("Vector can be reassigned") {
    auto vec = Vec3i{1, 2, 3};
    vec      = {2, 3, 4};
    CHECK(vec.x() == 2);
    CHECK(vec.y() == 3);
    CHECK(vec.z() == 4);
  }

  SECTION("Vector can be mem-copied") {
    const auto vec = Vec3i{4, 42, 1337};

    auto arr = std::array<int, 3>{};
    vec.memcpy(arr.data());
    CHECK(arr[0] == 4);
    CHECK(arr[1] == 42);
    CHECK(arr[2] == 1337);
  }

  SECTION("Vectors can be constructed from their components") {
    const auto vf = Vec3f{1.0, 2.0, 3.0};
    CHECK(vf.x() == 1.0);
    CHECK(vf.y() == 2.0);
    CHECK(vf.z() == 3.0);

    const auto vi = Vec3i{1, 2, 3};
    CHECK(vi.x() == 1);
    CHECK(vi.y() == 2);
    CHECK(vi.z() == 3);
  }

  SECTION("Vectors can be constructed from vectors of different types") {
    const auto vf = Vec3f{Vec3i{1, 2, 3}};
    CHECK(vf.x() == 1.0);
    CHECK(vf.y() == 2.0);
    CHECK(vf.z() == 3.0);

    const auto vi = Vec3i{Vec3f{1.0, 2.0, 3.0}};
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

  SECTION("Vectors can be iterated") {
    const auto vf = Vec3f{1.0, 2.0, 3.0};

    auto result = std::vector<float>{};
    for (float f : vf) {
      result.push_back(f);
    }
    CHECK(result == std::vector<float>{1.0, 2.0, 3.0});
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

  SECTION("Multipling vectors by other vectors multiplies each component") {
    CHECK(Vec3i{1, 2, 3} * Vec3i{2, 4, 2} == Vec3i{2, 8, 6});
  }

  SECTION("Dividing vectors by a scalar divides each component") {
    CHECK(Vec3i{2, 4, 6} / 2 == Vec3i{1, 2, 3});
  }

  SECTION("Dividing vectors by other vectors divides each component") {
    CHECK(Vec3i{2, 8, 4} / Vec3i{2, 4, 4} == Vec3i{1, 2, 1});
  }

  SECTION("Square magnitude is sum of squared components") {
    const auto vec = Vec3f{1.f, 2.f, 3.f};
    CHECK(approx(vec.getSqrMag(), 14.f));
  }

  SECTION("Magnitude is the square root of the sum of squared components") {
    const auto vec = Vec3f{0.f, 42.f, 0.f};
    CHECK(approx(vec.getMag(), 42.0f));
  }

  SECTION("Normalizing a vector results in a vector with length 1") {
    auto vec1 = Vec3f{0.f, 42.f, 0.f};
    CHECK(approx(vec1.getNorm(), dir3d::up()));

    vec1.norm();
    CHECK(approx(vec1, dir3d::up()));

    auto vec2 = Vec3f{1337.f, 42.f, -42.f};
    CHECK(approx(vec2.getNorm().getMag(), 1.f));

    vec2.norm();
    CHECK(approx(vec2.getMag(), 1.f));
  }

  SECTION("Dot product of the same vector is equal to the magnitude squared") {
    const auto vec1 = Vec2i{0, 4};
    CHECK(dot(vec1, vec1) == 16);

    const auto vec2 = Vec2i{-2, 0};
    CHECK(dot(vec2, vec2) == 4);

    const auto vec3 = Vec2i{-2, 3};
    CHECK(dot(vec3, vec3) == 13);
  }

  SECTION("Dot product of perpendicular unit vectors is 0") {
    const auto vec1 = Vec2f{0.f, 1.f};
    const auto vec2 = Vec2f{1.f, 0.f};
    CHECK(approxZero(dot(vec1, vec2)));
  }

  SECTION("Dot product of the same unit vector is 1") {
    const auto vec1 = Vec2f{0.f, 1.f};
    CHECK(approx(dot(vec1, vec1), 1.f));
  }

  SECTION("Dot product of unit vectors is the cosine of the angle between them") {
    const auto vec1 = Vec2f{0.f, 1.f};
    const auto vec2 = Vec2f{1.f, 1.f}.getNorm();
    CHECK(approx(std::acos(dot(vec1, vec2)) * radToDeg<float>, 45.0f));
  }

  SECTION("Dot product of opposite unit vectors is -1") {
    const auto vec1 = Vec2f{0.f, 1.f};
    const auto vec2 = Vec2f{0.f, -1.f};
    CHECK(approx(dot(vec1, vec2), -1.f));
  }

  SECTION("Cross of right and up is forward") {
    CHECK(approx(cross(dir3d::right(), dir3d::up()), dir3d::forward()));
  }

  SECTION("Cross of up and right is backward") {
    CHECK(approx(cross(dir3d::up(), dir3d::right()), dir3d::backward()));
  }

  SECTION("Cross of forward and right is up") {
    CHECK(approx(cross(dir3d::forward(), dir3d::right()), dir3d::up()));
  }

  SECTION("Cross of right and forward is down") {
    CHECK(approx(cross(dir3d::right(), dir3d::forward()), dir3d::down()));
  }

  SECTION("Angle between parallel vectors is 0 radians") {
    const auto vec1 = Vec2f{0.f, 1.f};
    const auto vec2 = Vec2f{0.f, 1.f};
    CHECK(approxZero(angle(vec1, vec2)));
  }

  SECTION("Angle between opposite vectors is pi radians") {
    const auto vec1 = Vec2f{0.f, 1.f};
    const auto vec2 = Vec2f{0.f, -1.f};
    CHECK(approx(angle(vec1, vec2), pi<float>));
  }

  SECTION("Angle between perpendicular vectors is half pi radians") {
    CHECK(approx(angle(Vec2f{0.f, 1.f}, Vec2f{1.f, 0.f}), pi<float> * .5f));
    CHECK(approx(angle(Vec2f{0.f, 1.f}, Vec2f{-1.f, 0.f}), pi<float> * .5f));
    CHECK(approx(angle(Vec2f{0.f, -1.f}, Vec2f{1.f, 0.f}), pi<float> * .5f));
    CHECK(approx(angle(Vec2f{0.f, -1.f}, Vec2f{-1.f, 0.f}), pi<float> * .5f));
  }

  SECTION("Angle between scaled vectors is the same an non-scaled vectors") {
    const auto vec1 = Vec2f{1.23f, -2.2f};
    const auto vec2 = Vec2f{-.23f, 4.42f};
    const auto a    = angle(vec1, vec2);
    CHECK(approx(angle(vec1.getNorm(), vec2.getNorm()), a));
  }

  SECTION("Angle between a vector and a zero-vector is zero") {
    const auto vec1 = Vec2f{1.23f, -2.2f};
    CHECK(approxZero(angle(vec1, Vec2f{})));
  }

  SECTION("Projecting a vector onto itself results in the same vector") {
    const auto vec = Vec2f{0.f, 1.f};
    CHECK(approx(project(vec, vec), vec));
  }

  SECTION("Projecting onto a vector of zero length returns a zero vector") {
    const auto vec = Vec2f{0.f, 1.f};
    CHECK(approxZero(project(vec, Vec2f{0.f, 0.f})));
  }

  SECTION("Projecting a zero vector returns a zero vector") {
    const auto vec = Vec2f{0.f, 0.f};
    CHECK(approxZero(project(vec, Vec2f{0.f, 1.f})));
  }

  SECTION("Projecting a vector onto another returns the 'overlap'") {
    const auto vec1 = Vec2f{3.f, 3.f};
    const auto vec2 = Vec2f{0.f, 10.f};
    CHECK(approx(project(vec1, vec2), Vec2f{0.f, 3.f}));
  }

  SECTION("Reflecting a zero vector returns a zero vector") {
    CHECK(approxZero(reflect(Vec2f{0.f, 0.f}, Vec2f{0.f, 1.f})));
  }

  SECTION("Reflecting a vector onto a zero vector returns itself") {
    const auto vec = Vec2f{5.0f, 1.f};
    CHECK(approx(reflect(vec, Vec2f{}), vec));
  }

  SECTION("Reflecting a vector onto an opposite normal reverses the direction") {
    CHECK(approx(reflect(Vec2f{5.0f, 1.f}, Vec2f{-1.f, 0.f}), Vec2f{-5.0f, 1.f}));
  }

  SECTION("Approx checks if vectors are approximately equal") {
    CHECK(approx(Vec2f{1.f, 2.0f}, Vec2f{1.f, 2.0f}));
    CHECK(approx(Vec2f{1.f, 2.0f}, Vec2f{1.f, 2.0000001f}));
    CHECK(!approx(Vec2f{1.f, 2.0f}, Vec2f{1.f, 2.001f}));
  }

  SECTION("ApproxZero check if a value is approximately zero") {
    CHECK(approxZero(Vec2f{0.f, 0.f}));
    CHECK(!approxZero(Vec2f{0.001f, 0.f}));
    CHECK(approxZero(Vec2f{0.0000001f, 0.f}));
  }

  SECTION("Approxing integers uses whole numbers") {
    CHECK(approx(Vec2i{1, 2}, Vec2i{1, 2}));
    CHECK(!approx(Vec2i{1, 2}, Vec2i{1, 3}));
    CHECK(approxZero(Vec2i{0, 0}));
    CHECK(!approxZero(Vec2i{0, 1}));
  }

  SECTION("lerp at value 0.5 returns the vector in the middle of x and y") {
    CHECK(lerp(Vec3i{10, 20, 10}, Vec3i{20, 40, 20}, 0.5) == Vec3i{15, 30, 15});
  }

  SECTION("persDivide divides each component by w") {
    CHECK(approx(persDivide(Vec4f{1.f, 2.f, 4.f, 4.f}), Vec3f{.25f, .5f, 1.f}));
  }

  SECTION("rndInsideUnitCube returns vectors where all components are <= 0.5 in magnitude") {
    auto rng = RngXorWow{42};
    for (auto i = 0U; i != 1'000; ++i) {
      auto p = rndInsideUnitCube3f(rng);
      REQUIRE(std::abs(p.x()) <= .5f);
      REQUIRE(std::abs(p.y()) <= .5f);
      REQUIRE(std::abs(p.z()) <= .5f);
    }
  }

  SECTION("rndOnUnitSphere returns unit vectors") {
    auto rng = RngXorWow{42};
    for (auto i = 0U; i != 1'000; ++i) {
      REQUIRE(approx(rndOnUnitSphere3f(rng).getSqrMag(), 1.f, .000001f));
    }
  }

  SECTION("rndInsideUnitSphere returns vectors of length 1 or less") {
    auto rng = RngXorWow{42};
    for (auto i = 0U; i != 1'000; ++i) {
      REQUIRE(rndInsideUnitSphere3f(rng).getSqrMag() <= 1.f);
    }
  }

  SECTION("Integers can be converted to colors") {
    // At runtime.
    for (auto i = 0U; i != 100; ++i) {
      auto col = color::get(i);
      CHECK((col.r() > 0 || col.g() > 0 || col.b() > 0));
      CHECK(approx(col.a(), 1.f));
    }

    // Or at compiletime.
    constexpr auto col = color::get(42);
    CHECK((col.r() > 0 || col.g() > 0 || col.b() > 0));
    CHECK(approx(col.a(), 1.f));
  }

  SECTION("Identical vectors produce identical hash values") {
    CHECK(
        std::hash<Vec3f>{}(Vec3f{1.337f, 42.0f, 0.1f}) ==
        std::hash<Vec3f>{}(Vec3f{1.337f, 42.0f, 0.1f}));
    CHECK(std::hash<Vec2i>{}(Vec2i{1, 2}) == std::hash<Vec2i>{}(Vec2i{1, 2}));
  }

  SECTION("Vectors map to a range of hash values") {
    // Note: This is not a good exhaustive check, instead just a quick sanity check.
    auto vecs = std::vector<Vec3f>{
        Vec3f{0.f, 1.2f, 0.5f},
        Vec3f{0.1f, 1.2f, 0.5f},
        Vec3f{0.2f, 1.2f, 0.5f},
        Vec3f{0.3f, 1.2f, 0.5f},
        Vec3f{0.4f, 1.2f, 0.5f},
    };
    auto hashes = std::unordered_set<size_t>{};
    for (const auto& vec : vecs) {
      auto hash = std::hash<Vec3f>{}(vec);
      CHECK(hashes.insert(hash).second);
    }
  }
}

} // namespace tria::math::tests

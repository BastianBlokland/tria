#include "catch2/catch.hpp"
#include "tria/math/mat.hpp"
#include "tria/math/mat_io.hpp"
#include "tria/math/utils.hpp"
#include "tria/math/vec.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Mat", "[math]") {

  SECTION("Matrices are fixed size") {
    auto m3 = Mat3f{};
    CHECK(sizeof(m3) == sizeof(float) * 3 * 3);

    auto m4 = Mat4f{};
    CHECK(sizeof(m4) == sizeof(float) * 4 * 4);
  }

  SECTION("Matrices can be reassigned") {
    auto m = Mat4f{};
    m      = identityMat4f();
    CHECK(m[0] == Vec4f{1.f, 0.f, 0.f, 0.f});
    CHECK(m[1] == Vec4f{0.f, 1.f, 0.f, 0.f});
    CHECK(m[2] == Vec4f{0.f, 0.f, 1.f, 0.f});
    CHECK(m[3] == Vec4f{0.f, 0.f, 0.f, 1.f});
  }

  SECTION("Matrices can be checked for equality") {
    CHECK(identityMat4f() == identityMat4f());
    CHECK(!(Mat4f{} == identityMat4f()));
  }

  SECTION("Vectors can be checked for inequality") {
    CHECK(Mat4f{} != identityMat4f());
    CHECK(!(identityMat4f() != identityMat4f()));
  }

  SECTION("Identity matrix multiplied by a identity matrix returns another identity matrix") {
    CHECK(approx(identityMat4f() * identityMat4f(), identityMat4f()));
  }

  SECTION("Matrix multiplication is the dot product of the rows and columns") {
    {
      auto mx = Mat2i{};
      mx[0]   = {1, 3};
      mx[1]   = {2, 4};

      auto my = Mat2i{};
      my[0]   = {2, 1};
      my[1]   = {0, 2};

      auto expected = Mat2i{};
      expected[0]   = {4, 10};
      expected[1]   = {4, 8};

      CHECK(mx * my == expected);
    }
    {
      auto mx = Mat2i{};
      mx[0]   = {2, 1};
      mx[1]   = {0, 2};

      auto my = Mat2i{};
      my[0]   = {1, 3};
      my[1]   = {2, 4};

      auto expected = Mat2i{};
      expected[0]   = {2, 7};
      expected[1]   = {4, 10};

      CHECK(mx * my == expected);
    }
  }

  SECTION("Multiply a matrix by a vector") {
    auto m = Mat3i{};
    m[0]   = {1, 0, 0};
    m[1]   = {-1, -3, 0};
    m[2]   = {2, 1, 1};

    auto v = Vec3i{2, 1, 0};

    CHECK(m * v == Vec3i{1, -3, 0});
  }

  SECTION("Transposing a matrix flip the rows and the columns") {
    auto m = Mat3i{};
    m[0]   = {1, 4, 7};
    m[1]   = {2, 5, 8};
    m[2]   = {3, 6, 9};

    auto t = Mat3i{};
    t[0]   = {1, 2, 3};
    t[1]   = {4, 5, 6};
    t[2]   = {7, 8, 9};

    CHECK(m.getTransposed() == t);
    CHECK(t.getTransposed() == m);
  }

  SECTION("Multiplying a matrix by the identity matrix yields the same matrix") {
    auto mx = Mat3f{};
    mx[0]   = {1.f, 4.f, 7.f};
    mx[1]   = {2.f, 5.f, 8.f};
    mx[2]   = {3.f, 6.f, 9.f};

    auto my = mx * identityMat3f();
    CHECK(approx(mx, my));
  }

  SECTION("Multiplying vector a by the identity matrix yields the same vector") {
    auto vx = Vec3f{2.f, 3.f, 4.f};
    auto vy = identityMat3f() * vx;
    CHECK(approx(vx, vy));
  }

  SECTION("Translation is applied as an offset to position vectors") {
    auto mat = transMat4f({-1.f, 2.f, .1f});
    CHECK(approx(mat * Vec4f{0.f, 0.f, 0.f, 1.f}, Vec4f{-1.f, 2.f, .1f, 1.f}));
    CHECK(approx(mat * Vec4f{1.f, 1.f, 1.f, 1.f}, Vec4f{0.f, 3.f, 1.1f, 1.f}));
    CHECK(approx(mat * Vec4f{-1.f, -1.f, -1.f, 1.f}, Vec4f{-2.f, 1.f, -.9f, 1.f}));
  }

  SECTION("Translation is ignored for direction vectors") {
    auto mat = transMat4f({-1.f, 2.f, .1f});
    CHECK(approx(mat * Vec4f{0.f, 0.f, 0.f, 0.f}, Vec4f{0.f, 0.f, 0.f, 0.f}));
    CHECK(approx(mat * Vec4f{1.f, 1.f, 1.f, 0.f}, Vec4f{1.f, 1.f, 1.f, 0.f}));
    CHECK(approx(mat * Vec4f{-1.f, -1.f, -1.f, 0.f}, Vec4f{-1.f, -1.f, -1.f, 0.f}));
  }

  SECTION("Scale is applied as a multiplier to position and direction vectors") {
    auto mat = scaleMat4f({1.f, 2.f, 3.f});
    CHECK(approx(mat * Vec4f{0.f, 0.f, 0.f, 1.f}, Vec4f{0.f, 0.f, 0.f, 1.f}));
    CHECK(approx(mat * Vec4f{1.f, 1.f, 1.f, 1.f}, Vec4f{1.f, 2.f, 3.f, 1.f}));
    CHECK(approx(mat * Vec4f{2.f, 3.f, 4.f, 1.f}, Vec4f{2.f, 6.f, 12.f, 1.f}));
    CHECK(approx(mat * Vec4f{2.f, 3.f, 4.f, 0.f}, Vec4f{2.f, 6.f, 12.f, 0.f}));

    CHECK(approx(scaleMat4f(3) * Vec4f{2.f, 3.f, 4.f, 1.f}, Vec4f{6.f, 9.f, 12.f, 1.f}));
    CHECK(approx(scaleMat4f(3) * Vec4f{2.f, 3.f, 4.f, 0.f}, Vec4f{6.f, 9.f, 12.f, 0.f}));
  }

  SECTION("Rotating 45 degrees on x results in a vector 45 degrees from the original") {
    auto ang = pi<float> * .25f; // 45 degrees.
    auto mat = rotXMat3f(ang);
    auto vec = Vec3f{0, -2.f, 3.f}.getNorm();
    CHECK(approx(angle(mat * vec, vec), ang));
  }

  SECTION("Rotating 180 degrees on x flips the axis") {
    auto angle = pi<float>; // 180 degrees.
    auto mat   = rotXMat3f(angle);
    CHECK(approx(mat * Vec3f{0.f, 1.f, 0.f}, Vec3f{0.f, -1.f, 0.f}));
  }

  SECTION("Rotating 45 degrees on y results in a vector 45 degrees from the original") {
    auto ang = pi<float> * .25f; // 45 degrees.
    auto mat = rotYMat3f(ang);
    auto vec = Vec3f{-2.f, 0.f, 3.f}.getNorm();
    CHECK(approx(angle(mat * vec, vec), ang));
  }

  SECTION("Rotating 180 degrees on y flips the axis") {
    auto angle = pi<float>; // 180 degrees.
    auto mat   = rotYMat3f(angle);
    CHECK(approx(mat * Vec3f{0.f, 0.f, 1.f}, Vec3f{0.f, 0.f, -1.f}));
  }

  SECTION("Rotating 45 degrees on z results in a vector 45 degrees from the original") {
    auto ang = pi<float> * .25f; // 45 degrees.
    auto mat = rotZMat3f(ang);
    auto vec = Vec3f{-2.f, 3.f, 0.f}.getNorm();
    CHECK(approx(angle(mat * vec, vec), ang));
  }

  SECTION("Rotating 180 degrees on z flips the axis") {
    auto angle = pi<float>; // 180 degrees.
    auto mat   = rotZMat3f(angle);
    CHECK(approx(mat * Vec3f{1.f, 0.f, 0.f}, Vec3f{-1.f, 0.f, 0.f}));
  }

  SECTION("Orthogonal projection scales to clip-space") {
    const auto size = Vec2f{10.f, 5.f};
    const auto proj = orthoProjMat4f(size, -2.f, 2.f);
    CHECK(approx(proj * Vec4f{0.f, 0.f, 0.f, 1.f}, Vec4f{0.f, 0.f, .5f, 1.f}));
    CHECK(approx(proj * Vec4f{+5.f, 0.f, 0.f, 1.f}, Vec4f{+1.f, 0.f, .5f, 1.f}));
    CHECK(approx(proj * Vec4f{-5.f, 0.f, 0.f, 1.f}, Vec4f{-1.f, 0.f, .5f, 1.f}));
    CHECK(approx(proj * Vec4f{-5.f, 5.f, 0.f, 1.f}, Vec4f{-1.f, -2.f, .5f, 1.f}));
    CHECK(approx(proj * Vec4f{-5.f, -5.f, 0.f, 1.f}, Vec4f{-1.f, 2.f, .5f, 1.f}));
    CHECK(approx(proj * Vec4f{-5.f, 0.f, -2.f, 1.f}, Vec4f{-1.f, 0.f, 0.f, 1.f}));
    CHECK(approx(proj * Vec4f{-5.f, 0.f, +2.f, 1.f}, Vec4f{-1.f, 0.f, 1.f, 1.f}));
  }

  SECTION("Perspective projection scales to clip-space") {
    const auto fov  = 90.f * math::degToRad<float>;
    const auto proj = persProjMat4f(fov, fov, 1.f, 10.f);
    CHECK(approx(persDivide(proj * Vec4f{0.f, 0.f, 1.f, 1.f}), Vec3f{0.f, 0.f, 0.f}));
    CHECK(approx(persDivide(proj * Vec4f{0.f, 0.f, 10.f, 1.f}), Vec3f{0.f, 0.f, 1.f}));
  }

  SECTION("Approx checks if two matrices are approximately equal") {
    CHECK(approx(identityMat4f(), identityMat4f()));

    auto mx  = Mat4f{};
    mx[1][2] = 2.f;

    auto my  = Mat4f{};
    my[1][2] = 2.0000001f;

    auto mz  = Mat4f{};
    mz[1][2] = 2.001f;

    CHECK(approx(mx, my));
    CHECK(!approx(mx, mz));
  }
}

} // namespace tria::math::tests

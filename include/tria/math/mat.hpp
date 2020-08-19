#pragma once
#include "tria/math/vec.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <type_traits>

namespace tria::math {

/* Square matrix.
 *
 * Column major and a left handed coordinate system.
 * - Positive x = right.
 * - Positive y = up.
 * - Positive z = 'into' the screen.
 *
 * Clip space:
 * - Output top left:     -1, -1
 * - Output top right:    +1, -1
 * - Output bottom left:  -1, +1
 * - Output bottom right: +1, +1
 * - Output depth: 0 - 1.
 */
template <typename Type, size_t Size>
class Mat final {
  static_assert(std::is_arithmetic_v<Type>, "Type has to be arithmetic");
  static_assert(std::is_trivially_destructible_v<Type>, "Type has to be trivially destructible");
  static_assert(std::is_trivially_copyable_v<Type>, "Type has to be trivially copyable");
  static_assert(std::is_standard_layout_v<Type>, "Type has to have a standard-layout");

public:
  using VecType    = Vec<Type, Size>;
  using ColumnType = VecType;
  using RowType    = VecType;

  constexpr Mat() noexcept : m_cols{} {}

  [[nodiscard]] constexpr auto getRow(size_t rowIdx) const noexcept -> RowType {
    RowType res;
    for (auto i = 0U; i != Size; ++i) {
      res[i] = m_cols[i][rowIdx];
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator[](size_t columnIdx) noexcept -> ColumnType& {
    return m_cols[columnIdx];
  }

  [[nodiscard]] constexpr auto operator[](size_t columnIdx) const noexcept -> const ColumnType& {
    return m_cols[columnIdx];
  }

  [[nodiscard]] constexpr auto operator==(const Mat<Type, Size>& rhs) const noexcept -> bool {
    return m_cols == rhs.m_cols;
  }

  [[nodiscard]] constexpr auto operator!=(const Mat<Type, Size>& rhs) const noexcept -> bool {
    return m_cols != rhs.m_cols;
  }

  [[nodiscard]] constexpr auto operator*(const Mat<Type, Size>& rhs) const noexcept
      -> Mat<Type, Size> {
    Mat<Type, Size> res;
    for (auto c = 0U; c != Size; ++c) {
      for (auto r = 0U; r != Size; ++r) {
        res[c][r] = dot(this->getRow(r), rhs[c]);
      }
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator*(const VecType& rhs) const noexcept -> VecType {
    VecType res;
    for (auto i = 0U; i != Size; ++i) {
      res[i] = dot(getRow(i), rhs);
    }
    return res;
  }

  [[nodiscard]] constexpr auto getTransposed() const noexcept -> Mat<Type, Size> {
    Mat<Type, Size> res;
    for (auto i = 0U; i != Size; ++i) {
      res[i] = getRow(i);
    }
    return res;
  }

private:
  std::array<ColumnType, Size> m_cols;
};

template <typename Type, size_t Size>
[[nodiscard]] constexpr auto identityMat() noexcept -> Mat<Type, Size> {
  auto res = Mat<Type, Size>{};
  for (auto i = 0U; i != Size; ++i) {
    res[i][i] = 1;
  }
  return res;
}

[[nodiscard]] constexpr auto identityMat2f() noexcept { return identityMat<float, 2>(); }
[[nodiscard]] constexpr auto identityMat3f() noexcept { return identityMat<float, 3>(); }
[[nodiscard]] constexpr auto identityMat4f() noexcept { return identityMat<float, 4>(); }

[[nodiscard]] constexpr auto identityMat2i() noexcept { return identityMat<int, 2>(); }
[[nodiscard]] constexpr auto identityMat3i() noexcept { return identityMat<int, 3>(); }
[[nodiscard]] constexpr auto identityMat4i() noexcept { return identityMat<int, 4>(); }

/* Translation matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto transMat4(Vec<T, 3> trans) noexcept {
  /*
   * [ 1,  0,  0,  x ]
   * [ 0,  1,  0,  y ]
   * [ 0,  0,  1,  z ]
   * [ 0,  0,  0,  1 ]
   */
  auto res  = identityMat<T, 4>();
  res[3][0] = trans.x();
  res[3][1] = trans.y();
  res[3][2] = trans.z();
  return res;
}

[[nodiscard]] constexpr auto transMat4f(Vec3f trans) noexcept { return transMat4(trans); }

/* Scale matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto scaleMat4(Vec<T, 3> scale) noexcept {
  /*
   * [ sx, 0,  0,  0 ]
   * [ 0,  sy, 0,  0 ]
   * [ 0,  0,  sz, 0 ]
   * [ 0,  0,  0,  1 ]
   */
  auto res  = identityMat<T, 4>();
  res[0][0] = scale.x();
  res[1][1] = scale.y();
  res[2][2] = scale.z();
  return res;
}

[[nodiscard]] constexpr auto scaleMat4f(Vec3f scale) noexcept { return scaleMat4(scale); }

/* Uniform scale matrix.
 */
[[nodiscard]] constexpr auto scaleMat4f(float scale) noexcept {
  return scaleMat4(Vec3f{1, 1, 1} * scale);
}

/* Rotate specfied amount of radians around the x axis.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto rotXMat(T angle) noexcept {
  static_assert(Size >= 3);
  /*
   * [ 1,  0,   0    ]
   * [ 0,  cos, -sin ]
   * [ 0,  sin, cos  ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, Size>();
  res[1][1] = c;
  res[1][2] = s;
  res[2][1] = -s;
  res[2][2] = c;
  return res;
}

[[nodiscard]] constexpr auto rotXMat3f(float angle) noexcept { return rotXMat<float, 3>(angle); }
[[nodiscard]] constexpr auto rotXMat4f(float angle) noexcept { return rotXMat<float, 4>(angle); }

/* Rotate specfied amount of radians around the y axis.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto rotYMat(T angle) noexcept {
  static_assert(Size >= 3);
  /*
   * [ cos,  0,  sin ]
   * [ 0,    1,  0   ]
   * [ -sin, 0,  cos ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, Size>();
  res[0][0] = c;
  res[0][2] = -s;
  res[2][0] = s;
  res[2][2] = c;
  return res;
}

[[nodiscard]] constexpr auto rotYMat3f(float angle) noexcept { return rotYMat<float, 3>(angle); }
[[nodiscard]] constexpr auto rotYMat4f(float angle) noexcept { return rotYMat<float, 4>(angle); }

/* Rotate specfied amount of radians around the z axis.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto rotZMat(T angle) noexcept {
  static_assert(Size >= 3);

  /*
   * [ cos, -sin, 0 ]
   * [ sin, cos,  0 ]
   * [ 0,   0,    1 ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, Size>();
  res[0][0] = c;
  res[0][1] = s;
  res[1][0] = -s;
  res[1][1] = c;
  return res;
}

[[nodiscard]] constexpr auto rotZMat3f(float angle) noexcept { return rotZMat<float, 3>(angle); }
[[nodiscard]] constexpr auto rotZMat4f(float angle) noexcept { return rotZMat<float, 4>(angle); }

/* Construct a rotation matrix from identity to the given axes.
 * Axis vectors have to be unit vectors and orthogonal to eachother.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto rotMat(Vec<T, 3> right, Vec<T, 3> up, Vec<T, 3> fwd) noexcept {
  static_assert(Size >= 3);

  // TODO(bastian): Should we just always normalize and reconstruct the axes? Reason for not doing
  // it atm is that the cost would be relatively high.
  assert(approx(right.getSqrMag(), T{1}, T{.0001}));
  assert(approx(up.getSqrMag(), T{1}, T{.0001}));
  assert(approx(fwd.getSqrMag(), T{1}, T{.0001}));
  assert(approxZero(dot(right, up)));

  /*
   * [ right.x,   up.x,   fwd.x ]
   * [ right.y,   up.y,   fwd.y ]
   * [ right.z,   up.z,   fwd.z ]
   */

  auto res  = identityMat<T, Size>();
  res[0][0] = right.x();
  res[0][1] = right.y();
  res[0][2] = right.z();
  res[1][0] = up.x();
  res[1][1] = up.y();
  res[1][2] = up.z();
  res[2][0] = fwd.x();
  res[2][1] = fwd.y();
  res[2][2] = fwd.z();
  return res;
}

[[nodiscard]] constexpr auto rotMat3f(Vec3f right, Vec3f up, Vec3f fwd) noexcept {
  return rotMat<float, 3>(right, up, fwd);
}
[[nodiscard]] constexpr auto rotMat4f(Vec3f right, Vec3f up, Vec3f fwd) noexcept {
  return rotMat<float, 4>(right, up, fwd);
}

/* Orthographic projection matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto orthoProjMat4(Vec<T, 2> size, T zNear, T zFar) noexcept {
  /*
   * [ 2 / w,       0,           0,           0            ]
   * [ 0,           -(2 / h),    0,           0            ]
   * [ 0,           0,           1 / (n - f), -f / (n - f) ]
   * [ 0,           0,           0,           1            ]
   *
   * Note: Setup for reversed-z depth so near objects are at depth 1 and far at 0.
   */
  auto res  = Mat<T, 4>{};
  res[0][0] = 2 / size.x();
  res[1][1] = -(2 / size.y());
  res[2][2] = 1 / (zNear - zFar);
  res[3][2] = -zFar / (zNear - zFar);
  res[3][3] = 1;
  return res;
}

[[nodiscard]] constexpr auto orthoProjMat4f(Vec2f size, float zNear, float zFar) noexcept {
  return orthoProjMat4(size, zNear, zFar);
}

/* Perspective projection matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto persProjMat4(T horAngle, T verAngle, T zNear) noexcept {
  /*
   * [ 1 / tan(hor / 2),  0,                    0,               0      ]
   * [ 0,                 -(1 / tan(ver / 2)),  0,               0      ]
   * [ 0,                 0,                    0,               zNear  ]
   * [ 0,                 0,                    1,               0      ]
   *
   * Note: Setup for reversed-z with an infinite far plane, so near objects are at depth 1 and depth
   * approaches 0 at infinite z.
   */
  auto res  = Mat<T, 4>{};
  res[0][0] = 1 / std::tan(horAngle * .5);
  res[1][1] = -(1 / std::tan(verAngle * .5));
  res[2][2] = 0;
  res[3][2] = zNear;
  res[2][3] = 1;
  return res;
}

[[nodiscard]] constexpr auto persProjMat4f(float horAngle, float verAngle, float zNear) noexcept {
  return persProjMat4(horAngle, verAngle, zNear);
}

[[nodiscard]] inline auto persProjVerMat4f(float verAngle, float aspect, float zNear) noexcept {
  const auto horAngle = std::atan(std::tan(verAngle * .5f) * aspect) * 2.f;
  return persProjMat4(horAngle, verAngle, zNear);
}

[[nodiscard]] inline auto persProjHorMat4f(float horAngle, float aspect, float zNear) noexcept {
  auto verAngle = std::atan(std::tan(horAngle * .5f) / aspect) * 2.f;
  return persProjMat4(horAngle, verAngle, zNear);
}

/* Check if all columns of two matrices are approximately equal.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto
approx(Mat<T, Size> x, Mat<T, Size> y, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Size; ++i) {
    if (!approx(x[i], y[i], maxDelta)) {
      return false;
    }
  }
  return true;
}

using Mat2f = Mat<float, 2>;
using Mat3f = Mat<float, 3>;
using Mat4f = Mat<float, 4>;

using Mat2i = Mat<int, 2>;
using Mat3i = Mat<int, 3>;
using Mat4i = Mat<int, 4>;

} // namespace tria::math

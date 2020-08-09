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
[[nodiscard]] constexpr auto transMat4(Vec<T, 3> translation) noexcept {
  /*
   * [ 1,  0,  0,  x ]
   * [ 0,  1,  0,  y ]
   * [ 0,  0,  1,  z ]
   * [ 0,  0,  0,  1 ]
   */
  auto res  = identityMat<T, 4>();
  res[3][0] = translation.x();
  res[3][1] = translation.y();
  res[3][2] = translation.z();
  return res;
}

/* Translation matrix.
 */
[[nodiscard]] constexpr auto transMat4f(Vec3f translation) noexcept {
  return transMat4(translation);
}

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

/* Scale matrix.
 */
[[nodiscard]] constexpr auto scaleMat4f(Vec3f scale) noexcept { return scaleMat4(scale); }

/* Uniform scale matrix.
 */
[[nodiscard]] constexpr auto scaleMat4f(float scale) noexcept {
  return scaleMat4(Vec3f{1, 1, 1} * scale);
}

/* Rotate specfied amount of radians around the x axis.
 */
template <typename T>
[[nodiscard]] constexpr auto rotXMat4(T angle) noexcept {
  /*
   * [ 1,  0,   0,    0 ]
   * [ 0,  cos, -sin, 0 ]
   * [ 0,  sin, cos,  0 ]
   * [ 0,  0,   0,    1 ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, 4>();
  res[1][1] = c;
  res[1][2] = s;
  res[2][1] = -s;
  res[2][2] = c;
  return res;
}

/* Rotate specfied amount of radians around the x axis.
 */
[[nodiscard]] constexpr auto rotXMat4f(float angle) noexcept { return rotXMat4(angle); }

/* Rotate specfied amount of radians around the y axis.
 */
template <typename T>
[[nodiscard]] constexpr auto rotYMat4(T angle) noexcept {
  /*
   * [ cos,  0,  sin, 0 ]
   * [ 0,    1,  0,   0 ]
   * [ -sin, 0,  cos, 0 ]
   * [ 0,    0,  0,   1 ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, 4>();
  res[0][0] = c;
  res[0][2] = -s;
  res[2][0] = s;
  res[2][2] = c;
  return res;
}

/* Rotate specfied amount of radians around the y axis.
 */
[[nodiscard]] constexpr auto rotYMat4f(float angle) noexcept { return rotYMat4(angle); }

/* Rotate specfied amount of radians around the z axis.
 */
template <typename T>
[[nodiscard]] constexpr auto rotZMat4(T angle) noexcept {
  /*
   * [ cos, -sin, 0,  0 ]
   * [ sin, cos,  0,  0 ]
   * [ 0,   0,    1,  0 ]
   * [ 0,   0,    0,  1 ]
   */
  auto c    = std::cos(angle);
  auto s    = std::sin(angle);
  auto res  = identityMat<T, 4>();
  res[0][0] = c;
  res[0][1] = s;
  res[1][0] = -s;
  res[1][1] = c;
  return res;
}

/* Rotate specfied amount of radians around the z axis.
 */
[[nodiscard]] constexpr auto rotZMat4f(float angle) noexcept { return rotZMat4(angle); }

/* Orthographic projection matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto orthoProjMat4(Vec<T, 2> size, T near, T far) noexcept {
  /*
   * [ 2 / w,       0,           0,           0            ]
   * [ 0,           -(2 / h),    0,           0            ]
   * [ 0,           0,           1 / (f - n), -n / (f - n) ]
   * [ 0,           0,           0,           1            ]
   */
  auto res  = Mat<T, 4>{};
  res[0][0] = 2 / size.x();
  res[1][1] = -(2 / size.y());
  res[2][2] = 1 / (far - near);
  res[3][2] = -near / (far - near);
  res[3][3] = 1;
  return res;
}

/* Orthographic projection matrix.
 */
[[nodiscard]] constexpr auto orthoProjMat4f(Vec2f size, float near, float far) noexcept {
  return orthoProjMat4(size, near, far);
}

/* Perspective projection matrix.
 */
template <typename T>
[[nodiscard]] constexpr auto persProjMat4(T horAngle, T verAngle, T near, T far) noexcept {
  /*
   * [ 1 / tan(hor / 2),  0,                    0,           0                ]
   * [ 0,                 -(1 / tan(ver / 2)),  0,           0                ]
   * [ 0,                 0,                    f / (f - n), -n * f / (f - n) ]
   * [ 0,                 0,                    1,           0                ]
   */
  auto res  = Mat<T, 4>{};
  res[0][0] = 1 / std::tan(horAngle * .5);
  res[1][1] = -(1 / std::tan(verAngle * .5));
  res[2][2] = far / (far - near);
  res[3][2] = -near * far / (far - near);
  res[2][3] = 1;
  res[3][3] = 0;
  return res;
}

/* Orthographic projection matrix.
 */
[[nodiscard]] constexpr auto
persProjMat4f(float horAngle, float verAngle, float near, float far) noexcept {
  return persProjMat4(horAngle, verAngle, near, far);
}

/* Orthographic projection matrix.
 */
[[nodiscard]] inline auto
persProjVerMat4f(float verAngle, float aspect, float near, float far) noexcept {
  const auto horAngle = std::atan(std::tan(verAngle * .5f) * aspect) * 2.f;
  return persProjMat4(horAngle, verAngle, near, far);
}

/* Orthographic projection matrix.
 */
[[nodiscard]] inline auto
persProjHorMat4f(float horAngle, float aspect, float near, float far) noexcept {
  auto verAngle = std::atan(std::tan(horAngle * .5f) / aspect) * 2.f;
  return persProjMat4(horAngle, verAngle, near, far);
}

/* Check if all columns of two matrices are approximately equal.
 * Note: Should not be used to compare to zero, use 'approxZero' instead.
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

/* Check if all columns of the given matrix are approximately zero.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto
approxZero(Mat<T, Size> x, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Size; ++i) {
    if (!approxZero(x[i], maxDelta)) {
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

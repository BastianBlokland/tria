#pragma once
#include "tria/math/mat.hpp"
#include "tria/math/utils.hpp"
#include "tria/math/vec.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <type_traits>

namespace tria::math {

/* Quaternion, used to represent a 3d rotation.
 */
template <typename Type>
class Quat final {
  static_assert(std::is_arithmetic_v<Type>, "Type has to be arithmetic");
  static_assert(std::is_trivially_destructible_v<Type>, "Type has to be trivially destructible");
  static_assert(std::is_trivially_copyable_v<Type>, "Type has to be trivially copyable");
  static_assert(std::is_standard_layout_v<Type>, "Type has to have a standard-layout");

public:
  constexpr static auto Size = 4U;

  constexpr Quat() noexcept : m_comps{} {}

  constexpr Quat(Type x, Type y, Type z, Type w) noexcept : m_comps{x, y, z, w} {}

  [[nodiscard]] constexpr auto operator[](size_t idx) noexcept -> Type& { return m_comps[idx]; }

  [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept -> const Type& {
    return m_comps[idx];
  }

  [[nodiscard]] constexpr auto x() noexcept -> Type& { return m_comps[0]; }
  [[nodiscard]] constexpr auto x() const noexcept -> const Type& { return m_comps[0]; }

  [[nodiscard]] constexpr auto y() noexcept -> Type& { return m_comps[1]; }
  [[nodiscard]] constexpr auto y() const noexcept -> const Type& { return m_comps[1]; }

  [[nodiscard]] constexpr auto z() noexcept -> Type& { return m_comps[2]; }
  [[nodiscard]] constexpr auto z() const noexcept -> const Type& { return m_comps[2]; }

  [[nodiscard]] constexpr auto w() noexcept -> Type& { return m_comps[3]; }
  [[nodiscard]] constexpr auto w() const noexcept -> const Type& { return m_comps[3]; }

  [[nodiscard]] constexpr auto operator==(const Quat<Type>& rhs) const noexcept -> bool {
    return m_comps == rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator!=(const Quat<Type>& rhs) const noexcept -> bool {
    return m_comps != rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator*(const Quat<Type>& rhs) const noexcept -> Quat<Type> {
    auto res = *this;
    res.x()  = w() * rhs.x() + x() * rhs.w() + y() * rhs.z() - z() * rhs.y();
    res.y()  = w() * rhs.y() + y() * rhs.w() + z() * rhs.x() - x() * rhs.z();
    res.z()  = w() * rhs.z() + z() * rhs.w() + x() * rhs.y() - y() * rhs.x();
    res.w()  = w() * rhs.w() - x() * rhs.x() - y() * rhs.y() - z() * rhs.z();
    return res;
  }

  [[nodiscard]] constexpr auto operator*(const Type& rhs) const noexcept -> Quat<Type> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] *= rhs;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator/(const Type& rhs) const noexcept -> Quat<Type> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] /= rhs;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator*(const Vec<Type, 3>& rhs) const noexcept -> Vec<Type, 3> {
    /* Implemenation based on:
    https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion */

    const auto vec    = Vec<Type, 3>{x(), y(), z()};
    const auto sqrMag = vec.getSqrMag();
    return vec * dot(vec, rhs) * 2 + rhs * (w() * w() - sqrMag) + cross(vec, rhs) * 2 * w();
  }

  [[nodiscard]] constexpr auto getInv() const noexcept -> Quat<Type> {
    auto res = *this;

    // Get the conjugate ('transposing').
    res.x() *= -1;
    res.y() *= -1;
    res.z() *= -1;

    // Divide by the squared length.
    // TODO(bastian): Should we just skip this? Is only needed for non-normalized quaternions.
    const auto sqrMag = res.getSqrMag();
    for (auto i = 0U; i != Size; ++i) {
      res.m_comps[i] /= sqrMag;
    }
    return res;
  }

  /* Calculate the magnitude of the quaternion squared.
   */
  [[nodiscard]] constexpr auto getSqrMag() const noexcept -> Type {
    Type res = {};
    for (auto i = 0U; i != Size; ++i) {
      res += m_comps[i] * m_comps[i];
    }
    return res;
  }

  /* Calculate the magnitude of the quaternion.
   */
  [[nodiscard]] constexpr auto getMag() const noexcept -> Type {
    auto sqrMag = getSqrMag();
    return sqrMag == 0 ? Type{} : std::sqrt(sqrMag);
  }

  /* Calculate a normalized version of this quaternion (magnitude of 1).
   * Note: If magnitude is 0 then the behaviour of this function is undefined.
   */
  [[nodiscard]] constexpr auto getNorm() const noexcept -> Quat<Type> {
    auto mag = getMag();
    assert(mag > std::numeric_limits<Type>::epsilon());
    return *this / mag;
  }

  /* Scale the quaternion so that the magnitude is 1.
   * Note: If magnitude is 0 then the behaviour of this function is undefined.
   */
  constexpr auto norm() noexcept {
    auto mag = getMag();
    assert(mag > std::numeric_limits<Type>::epsilon());
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] /= mag;
    }
  }

private:
  std::array<Type, Size> m_comps;
};

/* Identity quaternion, represents no rotation.
 */
template <typename Type>
[[nodiscard]] constexpr auto identityQuat() noexcept -> Quat<Type> {
  return {0, 0, 0, 1};
}

[[nodiscard]] constexpr auto identityQuatf() noexcept { return identityQuat<float>(); }

/* Rotate around an axis.
 * Angle is in radians.
 */
template <typename Type>
[[nodiscard]] constexpr auto angleAxisQuat(Vec<Type, 3> axis, Type angle) noexcept -> Quat<Type> {
  // TODO(bastian): Should we just document (and assume) that the axis should be a unit vector?
  const auto mag = axis.getMag();
  if (mag <= std::numeric_limits<Type>::epsilon()) {
    return identityQuat<Type>();
  }
  const auto vec = axis / mag * std::sin(angle * Type{.5});
  return {vec.x(), vec.y(), vec.z(), std::cos(angle * Type{.5})};
}

[[nodiscard]] constexpr auto angleAxisQuatf(Vec3f axis, float angle) noexcept {
  return angleAxisQuat<float>(axis, angle);
}

/* Construct a rotation matrix from a quaternion.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto rotMat(Quat<T> rot) noexcept {
  static_assert(Size >= 3);

  /*
   * [ 1 - 2y² - 2z²,   2xy + 2wz ,     2xz - 2wy,    ]
   * [ 2xy - 2wz,       1 - 2x² - 2z²,  2yz + 2wx,    ]
   * [ 2xz + 2wy,       2yz - 2wx,      1 - 2x² - 2y² ]
   */
  const auto& x = rot.x();
  const auto& y = rot.y();
  const auto& z = rot.z();
  const auto& w = rot.w();

  auto res  = identityMat<T, Size>();
  res[0][0] = 1 - 2 * y * y - 2 * z * z;
  res[0][1] = 2 * x * y + 2 * w * z;
  res[0][2] = 2 * x * z - 2 * w * y;
  res[1][0] = 2 * x * y - 2 * w * z;
  res[1][1] = 1 - 2 * x * x - 2 * z * z;
  res[1][2] = 2 * y * z + 2 * w * x;
  res[2][0] = 2 * x * z + 2 * w * y;
  res[2][1] = 2 * y * z - 2 * w * x;
  res[2][2] = 1 - 2 * x * x - 2 * y * y;
  return res;
}

[[nodiscard]] constexpr auto rotMat3f(Quat<float> rot) noexcept { return rotMat<float, 3>(rot); }
[[nodiscard]] constexpr auto rotMat4f(Quat<float> rot) noexcept { return rotMat<float, 4>(rot); }

/* Convert a rotation matrix to a quaternion.
 * Input matrix has to be an orthogonal matrix.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto quatFromMat(Mat<T, Size> mat) noexcept {
  static_assert(Size >= 3);

  /* Implemenation based on:
   * https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
   */
  Quat<T> res;
  const auto trace = mat[0][0] + mat[1][1] + mat[2][2]; // Sum of diag elements.
  if (trace > std::numeric_limits<T>::epsilon()) {
    // Trace is positive
    const auto s = std::sqrt(trace + 1.0f) * 2;
    res.x()      = (mat[1][2] - mat[2][1]) / s;
    res.y()      = (mat[2][0] - mat[0][2]) / s;
    res.z()      = (mat[0][1] - mat[1][0]) / s;
    res.w()      = s * .25;
  } else {
    // Trace zero or negative.
    // Find the biggest diagonal element.
    if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2]) {
      // [0, 0] is the biggest.
      const auto s = std::sqrt(1.0f + mat[0][0] - mat[1][1] - mat[2][2]) * 2;
      res.x()      = s * .25;
      res.y()      = (mat[1][0] + mat[0][1]) / s;
      res.z()      = (mat[2][0] + mat[0][2]) / s;
      res.w()      = (mat[1][2] - mat[2][1]) / s;
    } else if (mat[1][1] > mat[2][2]) {
      // [1, 1] is the biggest.
      const auto s = std::sqrt(1.0f + mat[1][1] - mat[0][0] - mat[2][2]) * 2;
      res.x()      = (mat[1][0] + mat[0][1]) / s;
      res.y()      = s * .25;
      res.z()      = (mat[2][1] + mat[1][2]) / s;
      res.w()      = (mat[2][0] - mat[0][2]) / s;
    } else {
      // [2, 2] is the biggest.
      const auto s = std::sqrt(1.0f + mat[2][2] - mat[0][0] - mat[1][1]) * 2;
      res.x()      = (mat[2][0] + mat[0][2]) / s;
      res.y()      = (mat[2][1] + mat[1][2]) / s;
      res.z()      = s * .25;
      res.w()      = (mat[0][1] - mat[1][0]) / s;
    }
  }
  return res;
}

[[nodiscard]] constexpr auto quatFromMat3f(Mat<float, 3> mat) noexcept { return quatFromMat(mat); }
[[nodiscard]] constexpr auto quatFromMat4f(Mat<float, 4> mat) noexcept { return quatFromMat(mat); }

/* Quaternion that rotates from the identity axes to the axis system formed by the given axes.
 * Vectors do not need to be normalized, but should not be zero.
 * Up does not need to be orthogonal to fwd as the up is reconstructed.
 */
template <typename Type>
[[nodiscard]] constexpr auto
lookRotQuat(const Vec<Type, 3> fwd, const Vec<Type, 3> upRef) noexcept {
  constexpr auto epsilon = std::numeric_limits<Type>::epsilon();

  // TODO(bastian): Should we just not bother and say calling this with zero vectors is undefined?
  if (fwd.getSqrMag() <= epsilon || upRef.getSqrMag() <= epsilon) {
    return identityQuat<Type>();
  }

  const auto dirFwd   = fwd.getNorm();
  const auto dirRight = cross(upRef, dirFwd).getNorm();
  const auto dirUp    = cross(dirFwd, dirRight);
  const auto mat      = rotMat<Type, 3>(dirRight, dirUp, dirFwd);
  return quatFromMat(mat);
}

[[nodiscard]] constexpr auto lookRotQuatf(Vec3f fwd, Vec3f up) noexcept {
  return lookRotQuat<float>(fwd, up);
}

/* Check if all components of two quaternions are approximately equal.
 */
template <typename T>
[[nodiscard]] constexpr auto approx(
    const Quat<T>& x, const Quat<T>& y, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Quat<T>::Size; ++i) {
    if (!approx(x[i], y[i], maxDelta)) {
      return false;
    }
  }
  return true;
}

using Quatf = Quat<float>;

} // namespace tria::math

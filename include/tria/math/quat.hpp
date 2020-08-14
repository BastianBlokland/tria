#pragma once
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

  [[nodiscard]] constexpr auto getSqrMag() const noexcept -> Type {
    Type res = {};
    for (auto i = 0U; i != Size; ++i) {
      res += m_comps[i] * m_comps[i];
    }
    return res;
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

/* Identity quaternion, represents no rotation.
 */
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

/* Rotate around an axis.
 * Angle is in radians.
 */
[[nodiscard]] constexpr auto angleAxisQuatf(Vec3f axis, float angle) noexcept {
  return angleAxisQuat<float>(axis, angle);
}

/* Check if all components of two quaternion are approximately equal.
 */
template <typename T>
[[nodiscard]] constexpr auto
approx(Quat<T> x, Quat<T> y, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Quat<T>::Size; ++i) {
    if (!approx(x[i], y[i], maxDelta)) {
      return false;
    }
  }
  return true;
}

using Quatf = Quat<float>;

} // namespace tria::math

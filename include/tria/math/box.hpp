#pragma once
#include "tria/math/utils.hpp"
#include "tria/math/vec.hpp"
#include <cassert>
#include <limits>
#include <type_traits>

namespace tria::math {

/* Axis Aligned Box.
 */
template <typename Type, size_t Size>
class Box final {
  static_assert(std::is_arithmetic_v<Type>, "Type has to be arithmetic");
  static_assert(std::is_trivially_destructible_v<Type>, "Type has to be trivially destructible");
  static_assert(std::is_trivially_copyable_v<Type>, "Type has to be trivially copyable");
  static_assert(std::is_standard_layout_v<Type>, "Type has to have a standard-layout");

public:
  using VecType = Vec<Type, Size>;

  constexpr Box() noexcept : m_min{}, m_max{} {}

  constexpr Box(VecType min, VecType max) noexcept : m_min{min}, m_max{max} {}

  [[nodiscard]] constexpr auto min() noexcept -> VecType& { return m_min; }
  [[nodiscard]] constexpr auto min() const noexcept -> const VecType& { return m_min; }

  [[nodiscard]] constexpr auto max() noexcept -> VecType& { return m_max; }
  [[nodiscard]] constexpr auto max() const noexcept -> const VecType& { return m_max; }

  [[nodiscard]] constexpr auto operator==(const Box& rhs) const noexcept -> bool {
    return m_min == rhs.m_min && m_max == rhs.m_max;
  }

  [[nodiscard]] constexpr auto operator!=(const Box& rhs) const noexcept -> bool {
    return m_min != rhs.m_min && m_max != rhs.m_max;
  }

  [[nodiscard]] constexpr auto getCenter() const noexcept -> VecType {
    return (m_min + m_max) * .5;
  }

  [[nodiscard]] constexpr auto getSize() const noexcept -> VecType { return m_max - m_min; }

  /* Grow the bounding box to encapsulate the given point.
   */
  constexpr auto encapsulate(const VecType& point) -> void {
    for (auto i = 0U; i != Size; ++i) {
      if (point[i] < m_min[i]) {
        m_min[i] = point[i];
      }
      if (point[i] > m_max[i]) {
        m_max[i] = point[i];
      }
    }
  }

private:
  VecType m_min;
  VecType m_max;
};

/* 'Inside out' box (infinitely small), usefull as a starting point for encapsulating points.
 */
template <typename Type, size_t Size>
[[nodiscard]] constexpr auto invertedBox() noexcept -> Box<Type, Size> {
  using lim = std::numeric_limits<Type>;
  Vec<Type, Size> min;
  for (auto i = 0U; i != Size; ++i) {
    min[i] = lim::max();
  }
  Vec<Type, Size> max;
  for (auto i = 0U; i != Size; ++i) {
    max[i] = lim::lowest();
  }
  return Box<Type, Size>{min, max};
}

[[nodiscard]] constexpr auto invertedBox2f() noexcept { return invertedBox<float, 2>(); }
[[nodiscard]] constexpr auto invertedBox3f() noexcept { return invertedBox<float, 3>(); }

/* Check if two axis-aligned-bounding-boxes are approximately equal.
 * Note: Should not be used to compare to zero, use 'approxZero' instead.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto approx(
    const Box<T, Size>& x,
    const Box<T, Size>& y,
    T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  return approx(x.min(), y.min(), maxDelta) && approx(x.max(), y.max(), maxDelta);
}

/* Check if the given box is approximately zero.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto
approxZero(const Box<T, Size>& x, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  return approxZero(x.min(), maxDelta) && approxZero(x.max(), maxDelta);
}

using Box2f = Box<float, 2>;
using Box3f = Box<float, 3>;

} // namespace tria::math

#pragma once
#include "tria/math/utils.hpp"
#include <array>
#include <type_traits>

namespace tria::math {

template <unsigned int Count, typename Type>
class Vec final {
  static_assert(std::is_arithmetic<Type>::value, "Type has to be arithmetic");

public:
  template <typename... Components>
  constexpr Vec(const Components&... comps) noexcept : m_comps{static_cast<Type>(comps)...} {}

  [[nodiscard]] constexpr auto operator[](unsigned int idx) noexcept -> Type& {
    return m_comps[idx];
  }

  [[nodiscard]] constexpr auto operator[](unsigned int idx) const noexcept -> const Type& {
    return m_comps[idx];
  }

  template <size_t N>
  [[nodiscard]] constexpr auto get() noexcept -> Type& {
    return m_comps[N];
  }

  template <size_t N>
  [[nodiscard]] constexpr auto get() const noexcept -> const Type& {
    return m_comps[N];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 1>>
  [[nodiscard]] constexpr auto x() noexcept -> Type& {
    return m_comps[0];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 1>>
  [[nodiscard]] constexpr auto x() const noexcept -> const Type& {
    return m_comps[0];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 1>>
  [[nodiscard]] constexpr auto r() noexcept -> Type& {
    return m_comps[0];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 1>>
  [[nodiscard]] constexpr auto r() const noexcept -> const Type& {
    return m_comps[0];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 2>>
  [[nodiscard]] constexpr auto y() noexcept -> Type& {
    return m_comps[1];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 2>>
  [[nodiscard]] constexpr auto y() const noexcept -> const Type& {
    return m_comps[1];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 2>>
  [[nodiscard]] constexpr auto g() noexcept -> Type& {
    return m_comps[1];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 2>>
  [[nodiscard]] constexpr auto g() const noexcept -> const Type& {
    return m_comps[1];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 3>>
  [[nodiscard]] constexpr auto z() noexcept -> Type& {
    return m_comps[2];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 3>>
  [[nodiscard]] constexpr auto z() const noexcept -> const Type& {
    return m_comps[2];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 3>>
  [[nodiscard]] constexpr auto b() noexcept -> Type& {
    return m_comps[2];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 3>>
  [[nodiscard]] constexpr auto b() const noexcept -> const Type& {
    return m_comps[2];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 4>>
  [[nodiscard]] constexpr auto w() noexcept -> Type& {
    return m_comps[3];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 4>>
  [[nodiscard]] constexpr auto w() const noexcept -> const Type& {
    return m_comps[3];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 4>>
  [[nodiscard]] constexpr auto a() noexcept -> Type& {
    return m_comps[3];
  }

  template <unsigned int C = Count, typename = std::enable_if_t<C >= 4>>
  [[nodiscard]] constexpr auto a() const noexcept -> const Type& {
    return m_comps[3];
  }

  [[nodiscard]] constexpr auto operator==(const Vec<Count, Type>& rhs) const noexcept -> bool {
    return m_comps == rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator!=(const Vec<Count, Type>& rhs) const noexcept -> bool {
    return m_comps != rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator+(const Vec<Count, Type>& rhs) const noexcept
      -> Vec<Count, Type> {
    auto res = *this;
    for (auto i = 0U; i != Count; ++i) {
      res[i] += rhs[i];
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator-(const Vec<Count, Type>& rhs) const noexcept
      -> Vec<Count, Type> {
    auto res = *this;
    for (auto i = 0U; i != Count; ++i) {
      res[i] -= rhs[i];
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> Vec<Count, Type> {
    auto res = *this;
    for (auto i = 0U; i != Count; ++i) {
      res[i] *= -1;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator*(const Type& rhs) const noexcept -> Vec<Count, Type> {
    auto res = *this;
    for (auto i = 0U; i != Count; ++i) {
      res[i] *= rhs;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator/(const Type& rhs) const noexcept -> Vec<Count, Type> {
    auto res = *this;
    for (auto i = 0U; i != Count; ++i) {
      res[i] /= rhs;
    }
    return res;
  }

  /* Calculate the magnitude of the vector squared.
   */
  [[nodiscard]] constexpr auto getSqrMag() const noexcept -> Type {
    Type res = {};
    for (auto i = 0U; i != Count; ++i) {
      res += m_comps[i] * m_comps[i];
    }
    return res;
  }

  /* Calculate the magnitude of the vector.
   */
  [[nodiscard]] constexpr auto getMag() const noexcept -> Type {
    auto sqrMag = getSqrMag();
    return sqrMag == 0 ? Type{} : std::sqrt(sqrMag);
  }

  /* Calculate a normalized version of this vector (unit vector).
   * Note: If magnitude is 0 then call this function results in undefined behaviour.
   */
  [[nodiscard]] constexpr auto getNorm() const noexcept -> Vec<Count, Type> {
    auto mag = getMag();
    return *this / mag;
  }

private:
  std::array<Type, Count> m_comps;
};

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

using Vec2i = Vec<2, int>;
using Vec3i = Vec<3, int>;
using Vec4i = Vec<4, int>;

using Color = Vec<4, float>;

namespace position {

[[nodiscard]] constexpr inline auto zero() { return Vec3f{0.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto one() { return Vec3f{1.0, 1.0, 1.0}; }

} // namespace position

namespace direction {

[[nodiscard]] constexpr inline auto up() { return Vec3f{0.0, 1.0, 0.0}; }
[[nodiscard]] constexpr inline auto down() { return Vec3f{0.0, -1.0, 0.0}; }
[[nodiscard]] constexpr inline auto right() { return Vec3f{1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto left() { return Vec3f{-1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto forward() { return Vec3f{0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto backward() { return Vec3f{0.0, 0.0, -1.0}; }

} // namespace direction

namespace color {

[[nodiscard]] constexpr inline auto white() { return Color{1.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto silver() { return Color{0.75, 0.75, 0.75, 1.0}; }
[[nodiscard]] constexpr inline auto gray() { return Color{0.5, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto black() { return Color{0.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto red() { return Color{1.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto maroon() { return Color{0.5, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto yellow() { return Color{1.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto olive() { return Color{0.5, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto lime() { return Color{0.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto green() { return Color{0.0, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto aqua() { return Color{0.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto teal() { return Color{0.0, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto blue() { return Color{0.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto navy() { return Color{0.0, 0.0, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto fuchsia() { return Color{1.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto purple() { return Color{0.5, 0.0, 0.5, 1.0}; }

} // namespace color

} // namespace tria::math

/* Specialize tuple_size and tuple_element so we can use structured bindings for Vec.
 */
namespace std {

template <unsigned int Count, typename Type>
struct tuple_size<tria::math::Vec<Count, Type>> : std::integral_constant<size_t, Count> {};

template <std::size_t N, unsigned int Count, typename Type>
struct tuple_element<N, tria::math::Vec<Count, Type>> {
  using type = Type;
};

} // namespace std

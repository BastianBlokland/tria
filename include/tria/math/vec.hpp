#pragma once
#include "tria/math/utils.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <type_traits>

namespace tria::math {

template <typename Type, size_t Size>
class Vec final {
  static_assert(std::is_arithmetic<Type>::value, "Type has to be arithmetic");

public:
  template <typename... Components>
  constexpr Vec(const Components&... comps) noexcept : m_comps{static_cast<Type>(comps)...} {}

  [[nodiscard]] constexpr auto getSize() noexcept -> size_t { return Size; }
  [[nodiscard]] constexpr auto getByteSize() noexcept -> size_t { return sizeof(Type) * Size; };

  [[nodiscard]] constexpr auto begin() noexcept -> Type* { return m_comps.begin(); }
  [[nodiscard]] constexpr auto begin() const noexcept -> const Type* { return m_comps.begin(); }
  [[nodiscard]] constexpr auto end() noexcept -> Type* { return m_comps.end(); }
  [[nodiscard]] constexpr auto end() const noexcept -> const Type* { return m_comps.end(); }

  [[nodiscard]] constexpr auto operator[](size_t idx) noexcept -> Type& { return m_comps[idx]; }

  [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept -> const Type& {
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

  template <size_t S = Size, typename = std::enable_if_t<S >= 1>>
  [[nodiscard]] constexpr auto x() noexcept -> Type& {
    return m_comps[0];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 1>>
  [[nodiscard]] constexpr auto x() const noexcept -> const Type& {
    return m_comps[0];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 1>>
  [[nodiscard]] constexpr auto r() noexcept -> Type& {
    return m_comps[0];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 1>>
  [[nodiscard]] constexpr auto r() const noexcept -> const Type& {
    return m_comps[0];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 2>>
  [[nodiscard]] constexpr auto y() noexcept -> Type& {
    return m_comps[1];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 2>>
  [[nodiscard]] constexpr auto y() const noexcept -> const Type& {
    return m_comps[1];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 2>>
  [[nodiscard]] constexpr auto g() noexcept -> Type& {
    return m_comps[1];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 2>>
  [[nodiscard]] constexpr auto g() const noexcept -> const Type& {
    return m_comps[1];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 3>>
  [[nodiscard]] constexpr auto z() noexcept -> Type& {
    return m_comps[2];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 3>>
  [[nodiscard]] constexpr auto z() const noexcept -> const Type& {
    return m_comps[2];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 3>>
  [[nodiscard]] constexpr auto b() noexcept -> Type& {
    return m_comps[2];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 3>>
  [[nodiscard]] constexpr auto b() const noexcept -> const Type& {
    return m_comps[2];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 4>>
  [[nodiscard]] constexpr auto w() noexcept -> Type& {
    return m_comps[3];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 4>>
  [[nodiscard]] constexpr auto w() const noexcept -> const Type& {
    return m_comps[3];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 4>>
  [[nodiscard]] constexpr auto a() noexcept -> Type& {
    return m_comps[3];
  }

  template <size_t S = Size, typename = std::enable_if_t<S >= 4>>
  [[nodiscard]] constexpr auto a() const noexcept -> const Type& {
    return m_comps[3];
  }

  [[nodiscard]] constexpr auto operator==(const Vec<Type, Size>& rhs) const noexcept -> bool {
    return m_comps == rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator!=(const Vec<Type, Size>& rhs) const noexcept -> bool {
    return m_comps != rhs.m_comps;
  }

  [[nodiscard]] constexpr auto operator+(const Vec<Type, Size>& rhs) const noexcept
      -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] += rhs[i];
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator-(const Vec<Type, Size>& rhs) const noexcept
      -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] -= rhs[i];
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] *= -1;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator*(const Type& rhs) const noexcept -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] *= rhs;
    }
    return res;
  }

  [[nodiscard]] constexpr auto operator/(const Type& rhs) const noexcept -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] /= rhs;
    }
    return res;
  }

  /* Calculate the magnitude of the vector squared.
   */
  [[nodiscard]] constexpr auto getSqrMag() const noexcept -> Type {
    Type res = {};
    for (auto i = 0U; i != Size; ++i) {
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
  [[nodiscard]] constexpr auto getNorm() const noexcept -> Vec<Type, Size> {
    auto mag = getMag();
    return *this / mag;
  }

  /* Mem-copy the values to a destination pointer.
   * Note: care must be taken that the output buffer is at least 'getSize()' elements big.
   */
  constexpr auto memcpy(Type* outPtr) const noexcept -> Type* {
    assert(outPtr);
    std::memcpy(outPtr, begin(), sizeof(Type) * Size);
    return outPtr;
  }

private:
  std::array<Type, Size> m_comps;
};

/* Return the linearly interpolated vector from x to y at time t.
 * Note: Does not clamp t (so can extrapolate too).
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto lerp(Vec<T, Size> x, Vec<T, Size> y, float t) noexcept
    -> Vec<T, Size> {
  Vec<T, Size> res = {};
  for (auto i = 0U; i != Size; ++i) {
    res[i] = lerp(x[i], y[i], t);
  }
  return res;
}

using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;

using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;

using Color = Vec<float, 4>;

namespace position {

[[nodiscard]] constexpr inline auto zero() noexcept { return Vec3f{0.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto one() noexcept { return Vec3f{1.0, 1.0, 1.0}; }

} // namespace position

namespace direction {

[[nodiscard]] constexpr inline auto up() noexcept { return Vec3f{0.0, 1.0, 0.0}; }
[[nodiscard]] constexpr inline auto down() noexcept { return Vec3f{0.0, -1.0, 0.0}; }
[[nodiscard]] constexpr inline auto right() noexcept { return Vec3f{1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto left() noexcept { return Vec3f{-1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr inline auto forward() noexcept { return Vec3f{0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto backward() noexcept { return Vec3f{0.0, 0.0, -1.0}; }

} // namespace direction

namespace color {

[[nodiscard]] constexpr inline auto white() noexcept { return Color{1.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto black() noexcept { return Color{0.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto clear() noexcept { return Color{0.0, 0.0, 0.0, 0.0}; }

[[nodiscard]] constexpr inline auto silver() noexcept { return Color{0.75, 0.75, 0.75, 1.0}; }
[[nodiscard]] constexpr inline auto gray() noexcept { return Color{0.5, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto red() noexcept { return Color{1.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto maroon() noexcept { return Color{0.5, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto yellow() noexcept { return Color{1.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto olive() noexcept { return Color{0.5, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto lime() noexcept { return Color{0.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto green() noexcept { return Color{0.0, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr inline auto aqua() noexcept { return Color{0.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto teal() noexcept { return Color{0.0, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto blue() noexcept { return Color{0.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto navy() noexcept { return Color{0.0, 0.0, 0.5, 1.0}; }
[[nodiscard]] constexpr inline auto fuchsia() noexcept { return Color{1.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr inline auto purple() noexcept { return Color{0.5, 0.0, 0.5, 1.0}; }

/* Get a color based on a unsigned integer, usefull for getting a color in debug code.
 */
[[nodiscard]] constexpr inline auto get(unsigned int i) noexcept -> Color {
  constexpr auto generators = std::array<Color (*)(), 14>{
      silver,
      gray,
      red,
      maroon,
      yellow,
      olive,
      lime,
      green,
      aqua,
      teal,
      blue,
      navy,
      fuchsia,
      purple};
  return generators[i % generators.size()]();
}

} // namespace color

} // namespace tria::math

/* Specialize tuple_size and tuple_element so we can use structured bindings for Vec.
 */
namespace std {

template <typename Type, size_t Size>
struct tuple_size<tria::math::Vec<Type, Size>> : std::integral_constant<size_t, Size> {};

template <std::size_t N, typename Type, size_t Size>
struct tuple_element<N, tria::math::Vec<Type, Size>> {
  using type = Type;
};

} // namespace std

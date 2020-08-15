#pragma once
#include "tria/log/param.hpp"
#include "tria/math/rnd.hpp"
#include "tria/math/utils.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <limits>
#include <type_traits>

namespace tria::math {

template <typename Type, size_t Size>
class Vec final {
  static_assert(std::is_arithmetic_v<Type>, "Type has to be arithmetic");
  static_assert(std::is_trivially_destructible_v<Type>, "Type has to be trivially destructible");
  static_assert(std::is_trivially_copyable_v<Type>, "Type has to be trivially copyable");
  static_assert(std::is_standard_layout_v<Type>, "Type has to have a standard-layout");

public:
  constexpr Vec() noexcept : m_comps{} {}

  template <typename OtherType>
  constexpr Vec(const Vec<OtherType, Size>& rhs) noexcept {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] = static_cast<Type>(rhs[i]);
    }
  }

  template <
      typename... Components,
      std::enable_if_t<sizeof...(Components) == Size, void*> = nullptr>
  constexpr Vec(const Components&... comps) noexcept : m_comps{static_cast<Type>(comps)...} {}

  [[nodiscard]] constexpr auto getSize() noexcept -> size_t { return Size; }

  [[nodiscard]] constexpr auto begin() noexcept -> Type* { return m_comps.data(); }
  [[nodiscard]] constexpr auto begin() const noexcept -> const Type* { return m_comps.data(); }
  [[nodiscard]] constexpr auto end() noexcept -> Type* { return m_comps.data() + Size; }
  [[nodiscard]] constexpr auto end() const noexcept -> const Type* { return m_comps.data() + Size; }

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

  constexpr auto operator+=(const Vec<Type, Size>& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] += rhs[i];
    }
    return *this;
  }

  [[nodiscard]] constexpr auto operator-(const Vec<Type, Size>& rhs) const noexcept
      -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] -= rhs[i];
    }
    return res;
  }

  constexpr auto operator-=(const Vec<Type, Size>& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] -= rhs[i];
    }
    return *this;
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

  constexpr auto operator*=(const Type& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] *= rhs;
    }
    return *this;
  }

  [[nodiscard]] constexpr auto operator*(const Vec<Type, Size>& rhs) const noexcept
      -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] *= rhs[i];
    }
    return res;
  }

  constexpr auto operator*=(const Vec<Type, Size>& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] *= rhs[i];
    }
    return *this;
  }

  [[nodiscard]] constexpr auto operator/(const Type& rhs) const noexcept -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] /= rhs;
    }
    return res;
  }

  constexpr auto operator/=(const Type& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] /= rhs;
    }
    return *this;
  }

  [[nodiscard]] constexpr auto operator/(const Vec<Type, Size>& rhs) const noexcept
      -> Vec<Type, Size> {
    auto res = *this;
    for (auto i = 0U; i != Size; ++i) {
      res[i] /= rhs[i];
    }
    return res;
  }

  constexpr auto operator/=(const Vec<Type, Size>& rhs) noexcept -> Vec<Type, Size>& {
    for (auto i = 0U; i != Size; ++i) {
      m_comps[i] /= rhs[i];
    }
    return *this;
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

/* Dot product of two vectors.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto dot(Vec<T, Size> x, Vec<T, Size> y) noexcept -> T {
  T res = {};
  for (auto i = 0U; i != Size; ++i) {
    res += x[i] * y[i];
  }
  return res;
}

/* Cross product of two 3d vectors.
 */
template <typename T>
[[nodiscard]] constexpr auto cross(Vec<T, 3> x, Vec<T, 3> y) noexcept -> Vec<T, 3> {
  return {
      (x.y() * y.z() - x.z() * y.y()),
      (x.z() * y.x() - x.x() * y.z()),
      (x.x() * y.y() - x.y() * y.x())};
}

/* Calculate the shortest angle in radians between the given vectors.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto angle(Vec<T, Size> from, Vec<T, Size> to) noexcept -> T {
  const auto denom = std::sqrt(from.getSqrMag() * to.getSqrMag());
  if (denom <= std::numeric_limits<T>::epsilon()) {
    return {};
  }
  return std::acos(std::clamp(dot(from, to) / denom, T{-1}, T{1}));
}

/* Project a vector onto another vector.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto project(Vec<T, Size> vec, Vec<T, Size> nrm) noexcept -> Vec<T, Size> {
  const auto nrmSqrMag = nrm.getSqrMag();
  if (nrmSqrMag <= std::numeric_limits<T>::epsilon()) {
    return {};
  }
  return nrm * dot(vec, nrm) / nrmSqrMag;
}

/* Reflect a vector off a normal.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto reflect(Vec<T, Size> vec, Vec<T, Size> nrm) noexcept -> Vec<T, Size> {
  return vec - nrm * dot(vec, nrm) * 2;
}

/* Return the linearly interpolated vector from x to y at time t.
 * Note: Does not clamp t (so can extrapolate too).
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto lerp(Vec<T, Size> x, Vec<T, Size> y, float t) noexcept
    -> Vec<T, Size> {
  Vec<T, Size> res;
  for (auto i = 0U; i != Size; ++i) {
    res[i] = lerp(x[i], y[i], t);
  }
  return res;
}

/* Perspective divide: divide the vector by its w component.
 */
template <typename T>
[[nodiscard]] constexpr auto persDivide(Vec<T, 4> vec) noexcept -> Vec<T, 3> {
  return Vec<T, 3>(vec.x(), vec.y(), vec.z()) / vec.w();
}

/* Check if all components of two vectors are approximately equal.
 * Note: Should not be used to compare to zero, use 'approxZero' instead.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto
approx(Vec<T, Size> x, Vec<T, Size> y, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Size; ++i) {
    if (!approx(x[i], y[i], maxDelta)) {
      return false;
    }
  }
  return true;
}

/* Check if all components of the given vector are approximately zero.
 */
template <typename T, size_t Size>
[[nodiscard]] constexpr auto
approxZero(Vec<T, Size> x, T maxDelta = std::numeric_limits<T>::epsilon()) noexcept {
  for (auto i = 0U; i != Size; ++i) {
    if (!approxZero(x[i], maxDelta)) {
      return false;
    }
  }
  return true;
}

/* Generate a random point inside a unit cube (all sides are 1 unit).
 */
template <typename Rng, typename VecT, size_t VecSize>
[[nodiscard]] auto rndInsideUnitCube(Rng& rng) noexcept {
  Vec<VecT, VecSize> res;
  for (auto i = 0U; i < VecSize; ++i) {
    res[i] = rndSample(rng, -.5f, +.5f);
  }
  return res;
}

/* Generate a random point inside a 2d unit cube (all sides are 1 unit).
 */
template <typename Rng>
[[nodiscard]] auto rndInsideUnitCube2f(Rng& rng) noexcept {
  return rndInsideUnitCube<Rng, float, 2>(rng);
}

/* Generate a random point inside a 3d unit cube (all sides are 1 unit).
 */
template <typename Rng>
[[nodiscard]] auto rndInsideUnitCube3f(Rng& rng) noexcept {
  return rndInsideUnitCube<Rng, float, 3>(rng);
}

/* Generate a random point on a unit sphere (radius of 1 unit, aka a direction vector).
 */
template <typename Rng, typename VecT, size_t VecSize>
[[nodiscard]] auto rndOnUnitSphere(Rng& rng) noexcept {
  // TODO(bastian): Should we just use a simple rejection method? Probably considerably faster.
  Vec<VecT, VecSize> res;
  while (true) {
    // Fill the vector with random numbers with a gaussian distribution.
    for (auto i = 0U; i < VecSize; i += 2) {
      const auto [g1, g2] = rndSampleGauss(rng);
      res[i]              = g1;
      // Note: because we generate 2 gaussian numbers at a time try to fill the next component with
      // the second value.
      if (i + 1 != VecSize) {
        res[i + 1] = g2;
      }
    }
    const auto sqrMag = res.getSqrMag();
    // Reject zero vectors (should be super rare).
    if (sqrMag <= std::numeric_limits<VecT>::epsilon()) {
      continue;
    }
    // Normalize to a unit vector.
    return res / std::sqrt(sqrMag);
  }
}

/* Generate a random point on a 2d unit sphere (radius of 1 unit, aka a direction vector).
 */
template <typename Rng>
[[nodiscard]] auto rndOnUnitSphere2f(Rng& rng) noexcept {
  return rndOnUnitSphere<Rng, float, 2>(rng);
}

/* Generate a random point on a 3d unit sphere (radius of 1 unit, aka a direction vector).
 */
template <typename Rng>
[[nodiscard]] auto rndOnUnitSphere3f(Rng& rng) noexcept {
  return rndOnUnitSphere<Rng, float, 3>(rng);
}

/* Generate a random point inside a unit sphere (radius of 1 unit).
 */
template <typename Rng, typename VecT, size_t VecSize>
[[nodiscard]] auto rndInsideUnitSphere(Rng& rng) noexcept {
  // TODO(bastian): Should we just use a simple rejection method? Probably considerably faster.
  // Cube-root as the area increases cubicly as you get further from the center.
  return rndOnUnitSphere<Rng, VecT, VecSize>(rng) * std::cbrt(rndSample(rng));
}

/* Generate a random point inside a 2d unit sphere (radius of 1 unit).
 */
template <typename Rng>
[[nodiscard]] auto rndInsideUnitSphere2f(Rng& rng) noexcept {
  return rndInsideUnitSphere<Rng, float, 2>(rng);
}

/* Generate a random point inside a 3d unit sphere (radius of 1 unit).
 */
template <typename Rng>
[[nodiscard]] auto rndInsideUnitSphere3f(Rng& rng) noexcept {
  return rndInsideUnitSphere<Rng, float, 3>(rng);
}

using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;

using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;

using Color = Vec<float, 4>;

namespace dir2d {

[[nodiscard]] constexpr auto up() noexcept { return Vec2f{0.0, 1.0}; }
[[nodiscard]] constexpr auto down() noexcept { return Vec2f{0.0, -1.0}; }
[[nodiscard]] constexpr auto right() noexcept { return Vec2f{1.0, 0.0}; }
[[nodiscard]] constexpr auto left() noexcept { return Vec2f{-1.0, 0.0}; }

} // namespace dir2d

namespace dir3d {

[[nodiscard]] constexpr auto up() noexcept { return Vec3f{0.0, 1.0, 0.0}; }
[[nodiscard]] constexpr auto down() noexcept { return Vec3f{0.0, -1.0, 0.0}; }
[[nodiscard]] constexpr auto right() noexcept { return Vec3f{1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr auto left() noexcept { return Vec3f{-1.0, 0.0, 0.0}; }
[[nodiscard]] constexpr auto forward() noexcept { return Vec3f{0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto backward() noexcept { return Vec3f{0.0, 0.0, -1.0}; }

} // namespace dir3d

namespace color {

[[nodiscard]] constexpr auto white() noexcept { return Color{1.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr auto black() noexcept { return Color{0.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto clear() noexcept { return Color{0.0, 0.0, 0.0, 0.0}; }

[[nodiscard]] constexpr auto silver() noexcept { return Color{0.75, 0.75, 0.75, 1.0}; }
[[nodiscard]] constexpr auto gray() noexcept { return Color{0.5, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr auto red() noexcept { return Color{1.0, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto maroon() noexcept { return Color{0.5, 0.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto yellow() noexcept { return Color{1.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto olive() noexcept { return Color{0.5, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr auto lime() noexcept { return Color{0.0, 1.0, 0.0, 1.0}; }
[[nodiscard]] constexpr auto green() noexcept { return Color{0.0, 0.5, 0.0, 1.0}; }
[[nodiscard]] constexpr auto aqua() noexcept { return Color{0.0, 1.0, 1.0, 1.0}; }
[[nodiscard]] constexpr auto teal() noexcept { return Color{0.0, 0.5, 0.5, 1.0}; }
[[nodiscard]] constexpr auto blue() noexcept { return Color{0.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr auto navy() noexcept { return Color{0.0, 0.0, 0.5, 1.0}; }
[[nodiscard]] constexpr auto fuchsia() noexcept { return Color{1.0, 0.0, 1.0, 1.0}; }
[[nodiscard]] constexpr auto purple() noexcept { return Color{0.5, 0.0, 0.5, 1.0}; }
[[nodiscard]] constexpr auto soothingPurple() noexcept { return Color{.188, .039, .141, 1.0}; }

/* Get a color based on a unsigned integer, usefull for getting a color in debug code.
 */
[[nodiscard]] constexpr auto get(unsigned int i) noexcept -> Color {
  constexpr auto colors = std::array<Color, 14>{
      red(),
      yellow(),
      olive(),
      silver(),
      aqua(),
      lime(),
      maroon(),
      blue(),
      teal(),
      navy(),
      fuchsia(),
      green(),
      gray(),
      purple(),
  };
  return colors[i % colors.size()];
}

} // namespace color

} // namespace tria::math

namespace std {

/* Specialize std::tuple_size and std::tuple_element so we can use structured bindings for vectors.
 */

template <typename Type, size_t Size>
struct tuple_size<tria::math::Vec<Type, Size>> : std::integral_constant<size_t, Size> {};

template <std::size_t N, typename Type, size_t Size>
struct tuple_element<N, tria::math::Vec<Type, Size>> {
  using type = Type;
};

/* Specialize std::hash to be able to use vectors as hash-map keys.
 */
template <typename Type, size_t Size>
struct hash<tria::math::Vec<Type, Size>> final {
  auto operator()(const tria::math::Vec<Type, Size>& vec) const noexcept -> size_t {
    size_t result = 0U;
    for (auto i = 0U; i != Size; ++i) {
      // Based on the boost::hash_combine, loads of good info in this stackoverflow question:
      // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes
      result ^= std::hash<Type>{}(vec[i]) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

/* Specialize tria::log::ValueFactory so vectors can be used as log parameters.
 */
namespace tria::log {

template <typename Type, size_t Size>
struct ValueFactory<math::Vec<Type, Size>> final {
  [[nodiscard]] auto operator()(math::Vec<Type, Size> vec) const noexcept -> std::vector<Value> {
    auto res = std::vector<Value>{};
    res.reserve(Size);
    for (auto i = 0U; i != Size; ++i) {
      res.emplace_back(vec[i]);
    }
    return res;
  }
};

} // namespace tria::log

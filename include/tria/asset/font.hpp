#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/vec.hpp"

namespace tria::asset {

enum class GlyphPointType {
  Normal,
  ControlPoint,
};

struct GlyphPoint final {
  math::Vec2f position;
  GlyphPointType type;

  constexpr GlyphPoint(math::Vec2f position, GlyphPointType type) :
      position{position}, type{type} {}

  [[nodiscard]] constexpr auto operator==(const GlyphPoint& rhs) const noexcept -> bool {
    return (position == rhs.position && type == rhs.type);
  }

  [[nodiscard]] constexpr auto operator!=(const GlyphPoint& rhs) const noexcept -> bool {
    return !operator==(rhs);
  }
};

class Glyph final {
public:
  Glyph(math::PodVector<uint16_t> contourEndPoints, math::PodVector<GlyphPoint> points) :
      m_contourEndPoints{std::move(contourEndPoints)}, m_points{std::move(points)} {}
  Glyph(const Glyph& rhs) = delete;
  Glyph(Glyph&& rhs)      = default;
  ~Glyph() noexcept       = default;

  auto operator=(const Glyph& rhs) -> Glyph& = delete;
  auto operator=(Glyph&& rhs) -> Glyph& = default;

  [[nodiscard]] auto getNumContours() const noexcept { return m_contourEndPoints.size(); }

  [[nodiscard]] auto getContourBegin(uint16_t c) const noexcept {
    assert(c < m_contourEndPoints.size());
    return m_points.data() + (c == 0U ? 0U : m_contourEndPoints[c - 1U]);
  }

  [[nodiscard]] auto getContourEnd(uint16_t c) const noexcept {
    assert(c < m_contourEndPoints.size());
    return m_points.data() + m_contourEndPoints[c];
  }

private:
  math::PodVector<uint16_t> m_contourEndPoints;
  math::PodVector<GlyphPoint> m_points;
};

/*
 * Asset containing font glyphs.
 */
class Font final : public Asset {
public:
  Font(AssetId id, std::vector<Glyph> glyphs) :
      Asset{std::move(id), getKind()}, m_glyphs{std::move(glyphs)} {}
  Font(const Font& rhs) = delete;
  Font(Font&& rhs)      = delete;
  ~Font() noexcept      = default;

  auto operator=(const Font& rhs) -> Font& = delete;
  auto operator=(Font&& rhs) -> Font& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Font; }

  [[nodiscard]] auto getGlyphCount() const noexcept { return m_glyphs.size(); }
  [[nodiscard]] auto getGlyphBegin() const noexcept { return m_glyphs.data(); }
  [[nodiscard]] auto getGlyphEnd() const noexcept { return m_glyphs.data() + m_glyphs.size(); }

private:
  std::vector<Glyph> m_glyphs;
};

/* Check if two glyph points are approximately equal.
 */
[[nodiscard]] constexpr auto approx(
    const GlyphPoint& x,
    const GlyphPoint& y,
    float maxDelta = std::numeric_limits<float>::epsilon()) noexcept {
  return approx(x.position, y.position, maxDelta) && x.type == y.type;
}

} // namespace tria::asset

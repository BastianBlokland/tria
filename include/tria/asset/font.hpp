#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/vec.hpp"

namespace tria::asset {

enum class GlyphSegmentType {
  Line,            // Consists of to 2 points, begin and end.
  QuadraticBezier, // Consists of 3 points, begin, control, end.
};

struct GlyphSegment final {
  GlyphSegmentType type;
  uint16_t startPointIdx; // Index of the first point, number of points depends on the type.

  constexpr GlyphSegment(GlyphSegmentType type, uint16_t startPointIdx) noexcept :
      type{type}, startPointIdx{startPointIdx} {}
};

class Glyph final {
public:
  Glyph(math::PodVector<math::Vec2f> points, math::PodVector<GlyphSegment> segments) :
      m_points{std::move(points)}, m_segments{std::move(segments)} {}
  Glyph(const Glyph& rhs) = delete;
  Glyph(Glyph&& rhs)      = default;
  ~Glyph() noexcept       = default;

  auto operator=(const Glyph& rhs) -> Glyph& = delete;
  auto operator=(Glyph&& rhs) -> Glyph& = default;

  [[nodiscard]] auto getNumSegments() const noexcept { return m_segments.size(); }
  [[nodiscard]] auto getSegmentsBegin() const noexcept { return m_segments.begin(); }
  [[nodiscard]] auto getSegmentsEnd() const noexcept { return m_segments.end(); }

  [[nodiscard]] auto getPoint(uint16_t idx) const noexcept {
    assert(idx < m_points.size());
    return m_points[idx];
  }

private:
  math::PodVector<math::Vec2f> m_points;
  math::PodVector<GlyphSegment> m_segments;
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

} // namespace tria::asset

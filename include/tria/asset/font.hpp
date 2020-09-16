#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/vec.hpp"
#include <unordered_map>

namespace tria::asset {

using CodePoint = uint32_t;

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
  Glyph(
      math::PodVector<CodePoint> codepoints,
      math::PodVector<math::Vec2f> points,
      math::PodVector<GlyphSegment> segments) :
      m_codepoints{std::move(codepoints)},
      m_points{std::move(points)},
      m_segments{std::move(segments)} {}
  Glyph(const Glyph& rhs)     = delete;
  Glyph(Glyph&& rhs) noexcept = default;
  ~Glyph() noexcept           = default;

  auto operator=(const Glyph& rhs) -> Glyph& = delete;
  auto operator=(Glyph&& rhs) noexcept -> Glyph& = default;

  [[nodiscard]] auto getNumCodepoints() const noexcept { return m_codepoints.size(); }
  [[nodiscard]] auto getCodepointsBegin() const noexcept { return m_codepoints.begin(); }
  [[nodiscard]] auto getCodepointsEnd() const noexcept { return m_codepoints.end(); }

  [[nodiscard]] auto getNumSegments() const noexcept { return m_segments.size(); }
  [[nodiscard]] auto getSegmentsBegin() const noexcept { return m_segments.begin(); }
  [[nodiscard]] auto getSegmentsEnd() const noexcept { return m_segments.end(); }

  [[nodiscard]] auto getPoint(uint16_t idx) const noexcept {
    assert(idx < m_points.size());
    return m_points[idx];
  }

private:
  math::PodVector<CodePoint> m_codepoints;
  math::PodVector<math::Vec2f> m_points;
  math::PodVector<GlyphSegment> m_segments;
};

/*
 * Asset containing font glyphs.
 */
class Font final : public Asset {
public:
  Font(AssetId id, std::vector<Glyph> glyphs) :
      Asset{std::move(id), getKind()}, m_glyphs{std::move(glyphs)} {
    // Construct a mapping from codepoints to their coresponding glyphs.
    for (const auto& g : m_glyphs) {
      for (auto cpItr = g.getCodepointsBegin(); cpItr != g.getCodepointsEnd(); ++cpItr) {
        m_lookup.insert({*cpItr, &g});
      }
    }
  }
  Font(const Font& rhs) = delete;
  Font(Font&& rhs)      = delete;
  ~Font() noexcept      = default;

  auto operator=(const Font& rhs) -> Font& = delete;
  auto operator=(Font&& rhs) -> Font& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Font; }

  [[nodiscard]] auto getGlyph(CodePoint cp) const noexcept -> const Glyph* {
    auto itr = m_lookup.find(cp);
    return itr == m_lookup.end() ? nullptr : itr->second;
  }

  [[nodiscard]] auto getGlyphCount() const noexcept { return m_glyphs.size(); }
  [[nodiscard]] auto getGlyphBegin() const noexcept { return m_glyphs.data(); }
  [[nodiscard]] auto getGlyphEnd() const noexcept { return m_glyphs.data() + m_glyphs.size(); }

private:
  std::unordered_map<CodePoint, const Glyph*> m_lookup;
  std::vector<Glyph> m_glyphs;
};

} // namespace tria::asset

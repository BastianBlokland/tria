#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/vec.hpp"

namespace tria::asset {

using IndexType = uint32_t;

/*
 * Per vertex data.
 */
struct Vertex final {
  math::Vec3f position;
  math::Vec3f normal;
  math::Vec4f tangent; // 'w' indicates the handedness, '1' or '-1'.
  math::Vec2f texcoord;

  constexpr Vertex() noexcept = default;
  constexpr Vertex(
      math::Vec3f position, math::Vec3f normal, math::Vec4f tangent, math::Vec2f texcoord) :
      position{position}, normal{normal}, tangent{tangent}, texcoord{texcoord} {}

  [[nodiscard]] constexpr auto operator==(const Vertex& rhs) const noexcept -> bool {
    return (
        position == rhs.position && normal == rhs.normal && rhs.tangent == rhs.tangent &&
        texcoord == rhs.texcoord);
  }

  [[nodiscard]] constexpr auto operator!=(const Vertex& rhs) const noexcept -> bool {
    return !operator==(rhs);
  }
};

/*
 * Asset containing geometry data.
 * Contains a set of vertices and indices that form primitives from the vertices.
 */
class Mesh final : public Asset {
public:
  Mesh(AssetId id, math::PodVector<Vertex> vertices, math::PodVector<IndexType> indices) :
      Asset{std::move(id), getKind()},
      m_vertices{std::move(vertices)},
      m_indices{std::move(indices)} {}
  Mesh(const Mesh& rhs) = delete;
  Mesh(Mesh&& rhs)      = delete;
  ~Mesh() noexcept      = default;

  auto operator=(const Mesh& rhs) -> Mesh& = delete;
  auto operator=(Mesh&& rhs) -> Mesh& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Mesh; }

  [[nodiscard]] auto getVertexCount() const noexcept { return m_vertices.size(); }
  [[nodiscard]] auto getVertexBegin() const noexcept { return m_vertices.begin(); }
  [[nodiscard]] auto getVertexEnd() const noexcept { return m_vertices.end(); }

  [[nodiscard]] auto getIndexCount() const noexcept { return m_indices.size(); }
  [[nodiscard]] auto getIndexBegin() const noexcept { return m_indices.begin(); }
  [[nodiscard]] auto getIndexEnd() const noexcept { return m_indices.end(); }

private:
  math::PodVector<Vertex> m_vertices;
  math::PodVector<IndexType> m_indices;
};

/* Check if two vertices are approximately equal.
 */
[[nodiscard]] constexpr auto approx(
    const Vertex& x,
    const Vertex& y,
    float maxDelta = std::numeric_limits<float>::epsilon()) noexcept {
  return (
      approx(x.position, y.position, maxDelta) && approx(x.normal, y.normal, maxDelta) &&
      approx(x.texcoord, y.texcoord, maxDelta));
}

} // namespace tria::asset

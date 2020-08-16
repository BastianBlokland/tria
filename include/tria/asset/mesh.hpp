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
  math::Vec2f texcoord;

  constexpr Vertex() noexcept = default;
  constexpr Vertex(math::Vec3f position, math::Vec3f normal, math::Vec2f texcoord) :
      position{position}, normal{normal}, texcoord{texcoord} {}

  [[nodiscard]] constexpr auto operator==(const Vertex& rhs) const noexcept -> bool {
    return position == rhs.position && normal == rhs.normal && texcoord == rhs.texcoord;
  }

  [[nodiscard]] constexpr auto operator!=(const Vertex& rhs) const noexcept -> bool {
    return !operator==(rhs);
  }
};

/*
 * Asset containing geometry data.
 * Contains a set of vertices and indices that form triangles from the vertices.
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

} // namespace tria::asset

namespace std {

/* Specialize std::hash to be able to use vertices as hash-map keys.
 */
template <>
struct hash<tria::asset::Vertex> final {
  auto operator()(const tria::asset::Vertex vert) const noexcept -> size_t {
    const auto posHash = std::hash<tria::math::Vec3f>{}(vert.position);
    const auto nrmHash = std::hash<tria::math::Vec3f>{}(vert.normal);
    return posHash ^ (nrmHash << 1);
  }
};

} // namespace std

#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/vec.hpp"
#include <vector>

namespace tria::asset {

using IndexType = uint16_t;

/*
 * Per vertex data.
 */
struct Vertex final {
  math::Vec3f position;
  math::Color color;

  constexpr Vertex(math::Vec3f position, math::Color color) : position{position}, color{color} {}

  [[nodiscard]] constexpr auto operator==(const Vertex& rhs) const noexcept -> bool {
    return position == rhs.position && color == rhs.color;
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
  Mesh(AssetId id, std::vector<Vertex> vertices, std::vector<IndexType> indices) :
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
  [[nodiscard]] auto getVertexBegin() const noexcept -> const Vertex* { return m_vertices.data(); }
  [[nodiscard]] auto getVertexEnd() const noexcept -> const Vertex* {
    return m_vertices.data() + m_vertices.size();
  }

  [[nodiscard]] auto getIndexCount() const noexcept { return m_indices.size(); }
  [[nodiscard]] auto getIndexBegin() const noexcept -> const IndexType* { return m_indices.data(); }
  [[nodiscard]] auto getIndexEnd() const noexcept -> const IndexType* {
    return m_indices.data() + m_indices.size();
  }

private:
  std::vector<Vertex> m_vertices;
  std::vector<IndexType> m_indices;
};

} // namespace tria::asset

namespace std {

/* Specialize std::hash to be able to use vertices as hash-map keys.
 */
template <>
struct hash<tria::asset::Vertex> final {
  auto operator()(const tria::asset::Vertex vert) const noexcept -> size_t {
    auto posHash = std::hash<tria::math::Vec3f>{}(vert.position);
    auto colHash = std::hash<tria::math::Color>{}(vert.color);
    return posHash ^ (colHash << 1);
  }
};

} // namespace std

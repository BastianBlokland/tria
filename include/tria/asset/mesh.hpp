#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/vec.hpp"
#include <vector>

namespace tria::asset {

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
 * Asset containing geometry data (vertices).
 */
class Mesh final : public Asset {
public:
  Mesh(AssetId id, std::vector<Vertex> vertices) :
      Asset{std::move(id), getKind()}, m_vertices{std::move(vertices)} {}
  Mesh(const Mesh& rhs) = delete;
  Mesh(Mesh&& rhs)      = delete;
  ~Mesh() noexcept      = default;

  auto operator=(const Mesh& rhs) -> Mesh& = delete;
  auto operator=(Mesh&& rhs) -> Mesh& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Mesh; }

  [[nodiscard]] auto getVertCount() const noexcept { return m_vertices.size(); }
  [[nodiscard]] auto getVertBegin() const noexcept -> const Vertex* { return m_vertices.data(); }
  [[nodiscard]] auto getVertEnd() const noexcept -> const Vertex* {
    return m_vertices.data() + m_vertices.size();
  }

private:
  std::vector<Vertex> m_vertices;
};

} // namespace tria::asset

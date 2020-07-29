#pragma once
#include "tria/asset/image.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/asset/shader.hpp"
#include <cassert>
#include <vector>

namespace tria::asset {

/*
 * Asset containing data needed for a drawing graphic.
 */
class Graphic final : public Asset {
public:
  Graphic(
      AssetId id,
      const Shader* vertShader,
      const Shader* fragShader,
      const Mesh* mesh,
      std::vector<const Image*> images) :
      Asset{std::move(id), getKind()},
      m_vertShader{vertShader},
      m_fragShader{fragShader},
      m_mesh{mesh},
      m_images{std::move(images)} {
    assert(m_vertShader);
    assert(m_fragShader);
    assert(m_mesh);
  }
  Graphic(const Graphic& rhs) = delete;
  Graphic(Graphic&& rhs)      = delete;
  ~Graphic() noexcept         = default;

  auto operator=(const Graphic& rhs) -> Graphic& = delete;
  auto operator=(Graphic&& rhs) -> Graphic& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Graphic; }

  [[nodiscard]] auto getVertShader() const noexcept { return m_vertShader; }
  [[nodiscard]] auto getFragShader() const noexcept { return m_fragShader; }
  [[nodiscard]] auto getMesh() const noexcept { return m_mesh; }

  [[nodiscard]] auto getImageCount() const noexcept { return m_images.size(); }
  [[nodiscard]] auto getImageBegin() const noexcept -> const Image* const* {
    return m_images.data();
  }
  [[nodiscard]] auto getImageEnd() const noexcept -> const Image* const* {
    return m_images.data() + m_images.size();
  }

private:
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;
  std::vector<const Image*> m_images;
};

} // namespace tria::asset

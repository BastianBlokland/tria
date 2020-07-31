#pragma once
#include "tria/asset/mesh.hpp"
#include "tria/asset/shader.hpp"
#include "tria/asset/texture.hpp"
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
      std::vector<const Texture*> textures) :
      Asset{std::move(id), getKind()},
      m_vertShader{vertShader},
      m_fragShader{fragShader},
      m_mesh{mesh},
      m_textures{std::move(textures)} {
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

  [[nodiscard]] auto getTextureCount() const noexcept { return m_textures.size(); }
  [[nodiscard]] auto getTextureBegin() const noexcept -> const Texture* const* {
    return m_textures.data();
  }
  [[nodiscard]] auto getTextureEnd() const noexcept -> const Texture* const* {
    return m_textures.data() + m_textures.size();
  }

private:
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;
  std::vector<const Texture*> m_textures;
};

} // namespace tria::asset

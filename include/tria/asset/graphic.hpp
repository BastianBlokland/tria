#pragma once
#include "tria/asset/mesh.hpp"
#include "tria/asset/shader.hpp"
#include "tria/asset/texture.hpp"
#include <cassert>
#include <vector>

namespace tria::asset {

/*
 * Contains a reference to a texture and sample settings.
 */
class TextureSampler final {
public:
  enum class FilterMode : uint8_t {
    Nearest = 0,
    Linear  = 1,
  };

  TextureSampler() = delete;
  TextureSampler(const Texture* texture, FilterMode filterMode) :
      m_texture{texture}, m_filterMode{filterMode} {
    assert(texture);
  }

  [[nodiscard]] auto getTexture() const noexcept { return m_texture; }
  [[nodiscard]] auto getFilterMode() const noexcept { return m_filterMode; }

private:
  const Texture* m_texture;
  FilterMode m_filterMode;
};

/*
 * Asset containing data needed for a drawing graphic.
 */
class Graphic final : public Asset {
public:
  enum class BlendMode : uint8_t {
    None          = 0,
    Alpha         = 1,
    Additive      = 2,
    AlphaAdditive = 3,
  };

  Graphic(
      AssetId id,
      const Shader* vertShader,
      const Shader* fragShader,
      const Mesh* mesh,
      std::vector<TextureSampler> samplers,
      BlendMode blendMode) :
      Asset{std::move(id), getKind()},
      m_vertShader{vertShader},
      m_fragShader{fragShader},
      m_mesh{mesh},
      m_samplers{std::move(samplers)},
      m_blendMode{blendMode} {
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

  [[nodiscard]] auto getSamplerCount() const noexcept { return m_samplers.size(); }
  [[nodiscard]] auto getSamplerBegin() const noexcept -> const TextureSampler* {
    return m_samplers.data();
  }
  [[nodiscard]] auto getSamplerEnd() const noexcept -> const TextureSampler* {
    return m_samplers.data() + m_samplers.size();
  }

  [[nodiscard]] auto getBlendMode() const noexcept { return m_blendMode; }

private:
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;
  std::vector<TextureSampler> m_samplers;
  BlendMode m_blendMode;
};

} // namespace tria::asset

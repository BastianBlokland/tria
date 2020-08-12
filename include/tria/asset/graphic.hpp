#pragma once
#include "tria/asset/mesh.hpp"
#include "tria/asset/shader.hpp"
#include "tria/asset/texture.hpp"
#include <cassert>
#include <vector>

namespace tria::asset {

/* Mode with which the fragment shader output color is blended with the framebuffer color.
 */
enum class BlendMode : uint8_t {
  None          = 0, // No blending, just replace the framebuffer's rgb values.
  Alpha         = 1, // Blend between the rgb values and the framebuffer based on the alpha.
  Additive      = 2, // Add the rgb values to the framebuffer (ignores alpha).
  AlphaAdditive = 3, // Multiply the rgb values by the alpha and add them to the framebuffer.
};

/* Mode that is used when oversampling the texture.
 */
enum class FilterMode : uint8_t {
  Nearest = 0, // Linearly blend between neighboring pixels.
  Linear  = 1, // Choose one of the pixels (sometimes known as 'point' filtering).
};

/* Mode that is used when performing depth-tests.
 */
enum class DepthTestMode : uint8_t {
  None = 0, // No depth-testing (always render).
  Less = 1, // Pass the depth-test if the fragment is closer.
};

/*
 * Contains a reference to a texture and sample settings.
 */
class TextureSampler final {
public:
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
  Graphic(
      AssetId id,
      const Shader* vertShader,
      const Shader* fragShader,
      const Mesh* mesh,
      std::vector<TextureSampler> samplers,
      BlendMode blendMode,
      DepthTestMode depthTestMode) :
      Asset{std::move(id), getKind()},
      m_vertShader{vertShader},
      m_fragShader{fragShader},
      m_mesh{mesh},
      m_samplers{std::move(samplers)},
      m_blendMode{blendMode},
      m_depthTestMode{depthTestMode} {
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
  [[nodiscard]] auto getDepthTestMode() const noexcept { return m_depthTestMode; }

private:
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;
  std::vector<TextureSampler> m_samplers;
  BlendMode m_blendMode;
  DepthTestMode m_depthTestMode;
};

} // namespace tria::asset

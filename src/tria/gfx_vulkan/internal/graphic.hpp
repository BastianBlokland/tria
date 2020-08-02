#pragma once
#include "asset_resource.hpp"
#include "device.hpp"
#include "sampler.hpp"
#include "transferer.hpp"
#include "tria/asset/graphic.hpp"
#include "tria/log/api.hpp"
#include "uniform_container.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Mesh;
class Texture;
class Shader;

/* Graphic resource.
 * Holding a vulkan pipeline and dependencies.
 */
class Graphic final {
public:
  using AssetType = asset::Graphic;

  Graphic(
      log::Logger* logger,
      Device* device,
      const asset::Graphic* asset,
      AssetResource<Shader>* shaders,
      AssetResource<Mesh>* meshes,
      AssetResource<Texture>* textures);
  Graphic(const Graphic& rhs) = delete;
  Graphic(Graphic&& rhs)      = delete;
  ~Graphic();

  auto operator=(const Graphic& rhs) -> Graphic& = delete;
  auto operator=(Graphic&& rhs) -> Graphic& = delete;

  /* Note: Call this before accessing any resources from this graphic.
   */
  auto
  prepareResources(Transferer* transferer, UniformContainer* uni, VkRenderPass vkRenderPass) const
      -> void;

  [[nodiscard]] auto getMesh() const noexcept { return m_mesh; }
  [[nodiscard]] auto getVkPipeline() const noexcept { return m_vkPipeline; }
  [[nodiscard]] auto getVkPipelineLayout() const noexcept { return m_vkPipelineLayout; }
  [[nodiscard]] auto getVkDescriptorSet() const noexcept { return m_descSet.getVkDescSet(); }

private:
  struct TextureData final {
    const Texture* texture;
    Sampler sampler;

    TextureData(const Texture* texture, Sampler sampler) :
        texture{texture}, sampler{std::move(sampler)} {}
  };

  log::Logger* m_logger;
  const Device* m_device;
  const asset::Graphic* m_asset;
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;

  DescriptorSet m_descSet;
  std::vector<TextureData> m_textures;

  mutable VkPipelineLayout m_vkPipelineLayout;
  mutable VkPipeline m_vkPipeline;
};

} // namespace tria::gfx::internal

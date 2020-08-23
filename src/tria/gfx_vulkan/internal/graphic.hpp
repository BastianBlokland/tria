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

constexpr auto g_shaderResourceGlobalSetId   = 0U; // Global resources.
constexpr auto g_shaderResourceGraphicSetId  = 1U; // Per graphic resources.
constexpr auto g_shaderResourceInstanceSetId = 2U; // Per instance resources.

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

  [[nodiscard]] auto getId() const noexcept { return m_asset->getId(); }

  /* Note: Call this before accessing any resources from this graphic.
   */
  auto prepareResources(
      Transferer* transferer,
      UniformContainer* uni,
      VkRenderPass vkRenderPass,
      VkSampleCount sampleCount) const -> void;

  [[nodiscard]] auto getMesh() const noexcept { return m_mesh; }
  [[nodiscard]] auto getUsesGlobalData() const noexcept { return m_usesGlobalData; }
  [[nodiscard]] auto getUsesInstanceData() const noexcept { return m_usesInstanceData; }
  [[nodiscard]] auto getVkPipeline() const noexcept { return m_vkPipeline; }
  [[nodiscard]] auto getVkPipelineLayout() const noexcept { return m_vkPipelineLayout; }
  [[nodiscard]] auto getVkDescriptorSet() const noexcept { return m_descSet.getVkDescSet(); }

private:
  struct TextureData final {
    const Texture* texture;
    Sampler sampler;
  };

  log::Logger* m_logger;
  const Device* m_device;
  const asset::Graphic* m_asset;
  std::vector<const Shader*> m_shaders;
  bool m_usesGlobalData;
  bool m_usesInstanceData;
  const Mesh* m_mesh;

  DescriptorSet m_descSet;
  std::vector<TextureData> m_textures;

  mutable VkPipelineLayout m_vkPipelineLayout;
  mutable VkPipeline m_vkPipeline;
};

} // namespace tria::gfx::internal

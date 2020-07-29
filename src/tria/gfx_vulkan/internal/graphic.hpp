#pragma once
#include "asset_resource.hpp"
#include "transferer.hpp"
#include "tria/asset/graphic.hpp"
#include "tria/log/api.hpp"
#include "uniform_container.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;
class Mesh;
class Image;
class Shader;

/* Graphic resource.
 * Holding a vulkan pipeline and dependencies.
 */
class Graphic final {
public:
  using AssetType = asset::Graphic;

  Graphic(
      log::Logger* logger,
      const Device* device,
      const asset::Graphic* asset,
      AssetResource<Shader>* shaders,
      AssetResource<Mesh>* meshes,
      AssetResource<Image>* images);
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

private:
  log::Logger* m_logger;
  const Device* m_device;
  const asset::Graphic* m_asset;
  const Shader* m_vertShader;
  const Shader* m_fragShader;
  const Mesh* m_mesh;
  std::vector<const Image*> m_images;
  mutable VkPipelineLayout m_vkPipelineLayout;
  mutable VkPipeline m_vkPipeline;
};

} // namespace tria::gfx::internal

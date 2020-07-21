#pragma once
#include "asset_resource.hpp"
#include "tria/asset/graphic.hpp"
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;
class Mesh;
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
      VkRenderPass vkRenderPass);
  Graphic(const Graphic& rhs) = delete;
  Graphic(Graphic&& rhs)      = delete;
  ~Graphic();

  auto operator=(const Graphic& rhs) -> Graphic& = delete;
  auto operator=(Graphic&& rhs) -> Graphic& = delete;

  [[nodiscard]] auto getMesh() const noexcept { return m_mesh; }
  [[nodiscard]] auto getVkPipeline() const noexcept { return m_vkPipeline; }

private:
  log::Logger* m_logger;
  const Device* m_device;
  const Mesh* m_mesh;
  VkPipelineLayout m_vkPipelineLayout;
  VkPipeline m_vkPipeline;
};

} // namespace tria::gfx::internal

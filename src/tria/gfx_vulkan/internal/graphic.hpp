#pragma once
#include "tria/asset/graphic.hpp"
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;
class ShaderManager;

class Graphic final {
public:
  Graphic(
      log::Logger* logger,
      const Device* device,
      ShaderManager* shaderManager,
      VkRenderPass vkRenderPass,
      const asset::Graphic* asset);
  Graphic(const Graphic& rhs) = delete;
  Graphic(Graphic&& rhs)      = delete;
  ~Graphic();

  auto operator=(const Graphic& rhs) -> Graphic& = delete;
  auto operator=(Graphic&& rhs) -> Graphic& = delete;

  [[nodiscard]] auto getVkPipeline() const noexcept { return m_vkPipeline; }

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkPipelineLayout m_vkPipelineLayout;
  VkPipeline m_vkPipeline;
};

} // namespace tria::gfx::internal

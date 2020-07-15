#pragma once
#include "device.hpp"
#include "tria/asset/graphic.hpp"
#include "tria/log/api.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Graphic final {
public:
  Graphic(
      log::Logger* logger,
      const Device* device,
      VkRenderPass vkRenderPass,
      const asset::Graphic* asset);
  ~Graphic();

  [[nodiscard]] auto getVkPipeline() const noexcept { return m_pipeline; }

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkShaderModule m_vertShader;
  VkShaderModule m_fragShader;
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_pipeline;
};

using GraphicUnique = std::unique_ptr<Graphic>;

} // namespace tria::gfx::internal

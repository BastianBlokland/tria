#pragma once
#include "image.hpp"
#include "swapchain.hpp"
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/* Technique that renders straight into a swapchain image.
 */
class ForwardTechnique final {
public:
  ForwardTechnique(Device* device);
  ForwardTechnique(const ForwardTechnique& rhs)     = delete;
  ForwardTechnique(ForwardTechnique&& rhs) noexcept = delete;
  ~ForwardTechnique();

  auto operator=(const ForwardTechnique& rhs) -> ForwardTechnique& = delete;
  auto operator=(ForwardTechnique&& rhs) noexcept -> ForwardTechnique& = delete;

  /* Render size.
   */
  [[nodiscard]] auto getSize() const noexcept { return m_size; }
  [[nodiscard]] auto getVkRenderPass() const noexcept { return m_vkRenderPass; }

  /* (Re)create any resources required for this technique.
   */
  auto prepareResources(const Swapchain& swapchain) -> void;

  auto
  beginRenderPass(VkCommandBuffer vkCommandBuffer, SwapchainIdx idx, math::Color clearCol) const
      noexcept -> void;

private:
  Device* m_device;
  SwapchainSize m_size;
  SwapchainVersion m_swapVersion;
  VkRenderPass m_vkRenderPass;
  std::vector<VkFramebuffer> m_vkFramebuffers;
};

using ForwardTechniqueUnique = std::unique_ptr<ForwardTechnique>;

} // namespace tria::gfx::internal

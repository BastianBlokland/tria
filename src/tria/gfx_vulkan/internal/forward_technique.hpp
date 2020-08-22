#pragma once
#include "image.hpp"
#include "swapchain.hpp"
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/* Technique that renders into a swapchain image.
 * When using multiple samples a intermediate target is used and the result is resolved into the
 * swapchain image.
 */
class ForwardTechnique final {
public:
  ForwardTechnique(Device* device, VkSampleCount sampleCount, DepthMode depth, ClearMask clear);
  ForwardTechnique(const ForwardTechnique& rhs)     = delete;
  ForwardTechnique(ForwardTechnique&& rhs) noexcept = delete;
  ~ForwardTechnique();

  auto operator=(const ForwardTechnique& rhs) -> ForwardTechnique& = delete;
  auto operator=(ForwardTechnique&& rhs) noexcept -> ForwardTechnique& = delete;

  /* Render size.
   */
  [[nodiscard]] auto getSize() const noexcept { return m_size; }
  [[nodiscard]] auto getVkRenderPass() const noexcept { return m_vkRenderPass; }
  [[nodiscard]] auto getSampleCount() const noexcept { return m_sampleCount; }

  /* (Re)create any resources required for this technique.
   */
  auto prepareResources(const Swapchain& swapchain) -> void;

  auto
  beginRenderPass(VkCommandBuffer vkCommandBuffer, SwapchainIdx idx, math::Color clearCol) const
      noexcept -> void;

private:
  Device* m_device;
  VkSampleCount m_sampleCount;
  DepthMode m_depth;
  SwapchainSize m_size;
  SwapchainVersion m_swapVersion;
  VkRenderPass m_vkRenderPass;
  Image m_colorTarget; // Optional: Only used when sampleCount > 1.
  Image m_depthTarget; // Optional: Only used when depth == DepthMode::Enable.
  std::vector<VkFramebuffer> m_vkFramebuffers;
};

using ForwardTechniqueUnique = std::unique_ptr<ForwardTechnique>;

} // namespace tria::gfx::internal

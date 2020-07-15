#pragma once
#include "tria/gfx/context.hpp"
#include "tria/log/api.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/*
 * Swapchain is responsible for managing the images that are presented to the surface (window).
 */
class Swapchain final {
public:
  Swapchain(log::Logger* logger, const Device* device, VSyncMode vSync);
  ~Swapchain();

  [[nodiscard]] auto getExtent() const noexcept { return m_extent; }
  [[nodiscard]] auto getVkFramebuffer(uint32_t imageIndex) const -> const VkFramebuffer&;

  /* Acquire a new image to render into.
   * Returns: an index that can be used to retrieve the framebuffer for that image.
   * Note: When function returns the image might not be ready yet for rendering into, but all cpu
   * side recording can already be done and the 'imgAvailable' sempahore will fire once the image is
   * ready.
   * Note: if we fail to acquire an image null-optional is returned (can happen when window
   * is minimized for example).
   */
  [[nodiscard]] auto
  acquireImage(VkRenderPass vkRenderPass, VkSemaphore imgAvailable, bool forceReinit)
      -> std::optional<uint32_t>;

  /* Present a rendered image to the surface (window).
   * Note: actual presenting will only occur when 'imgReady' semaphore is fired, so this can be
   * called before the rendering is done.
   */
  auto presentImage(VkSemaphore imgReady, uint32_t imageIndex) -> bool;

private:
  log::Logger* m_logger;
  const Device* m_device;
  VSyncMode m_vSync;
  unsigned int m_imgCount;
  VkExtent2D m_extent;
  VkSwapchainKHR m_vkSwapchain;
  std::vector<VkImage> m_vkImages;
  std::vector<VkImageView> m_vkImageViews;
  std::vector<VkFramebuffer> m_vkFramebuffers;
  bool m_swapchainOutOfDate;

  auto initSwapchain(VkRenderPass vkRenderPass) -> bool;
};

using SwapchainUnique = std::unique_ptr<Swapchain>;

} // namespace tria::gfx::internal

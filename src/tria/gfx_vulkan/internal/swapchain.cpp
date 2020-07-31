#include "swapchain.hpp"
#include "device.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getImageCount(const VkSurfaceCapabilitiesKHR& capabilities) noexcept
    -> uint32_t {
  // One more then minimum to avoid having to block to acquire a new image.
  auto result = capabilities.minImageCount + 1;
  // Note '0' maxImageCount indicates that there is no maximum.
  if (capabilities.maxImageCount != 0 && result > capabilities.maxImageCount) {
    result = capabilities.maxImageCount;
  }
  return result;
}

[[nodiscard]] auto getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) noexcept
    -> VkExtent2D {
  if (capabilities.currentExtent.width != UINT32_MAX &&
      capabilities.currentExtent.height != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    // Note: in this case the surface does not have a preference what size the swapchain should be.
    // Probably in this case we should use the size of the window, however i have not seen this
    // happen on real platforms.
    return capabilities.minImageExtent;
  }
}

[[nodiscard]] auto getPresentMode(const Device* device, VSyncMode vSync) noexcept
    -> VkPresentModeKHR {
  assert(device);
  const auto modes = getVkPresentModes(device->getVkPhysicalDevice(), device->getVkSurface());
  if (vSync == VSyncMode::Disable) {
    // Prefer mailbox.
    for (const auto& mode : modes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return mode;
      }
    }
    // Or immediate (tearing).
    for (const auto& mode : modes) {
      if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        return mode;
      }
    }
  }
  // Fifo (vsync) is guaranteed to be available.
  return VK_PRESENT_MODE_FIFO_KHR;
}

[[nodiscard]] auto createVkSwapchain(
    const Device* device,
    VkSwapchainKHR oldSwapchain,
    uint32_t imgCount,
    VkExtent2D extent,
    VkSurfaceTransformFlagBitsKHR transformFlags,
    VkPresentModeKHR presentMode) -> VkSwapchainKHR {
  assert(device);

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface                  = device->getVkSurface();
  createInfo.minImageCount            = imgCount;
  createInfo.imageFormat              = device->getVkSurfaceFormat().format;
  createInfo.imageColorSpace          = device->getVkSurfaceFormat().colorSpace;
  createInfo.imageExtent              = extent;
  createInfo.imageArrayLayers         = 1;
  createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array<uint32_t, 2> queueFamilyIndices = {
      device->getVkGraphicsQueueIdx(),
      device->getVkPresentQueueIdx(),
  };
  if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    // Note: this supports support devices where the graphics and present queues are not the
    // same, however this is not a common case. If it becomes a more common case we should probably
    // setup explicit ownership transfers instead of the concurrent mode for higher performance.
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
  }

  createInfo.preTransform   = transformFlags;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = true;
  createInfo.oldSwapchain   = oldSwapchain;

  // Create a new swapchain.
  VkSwapchainKHR result;
  checkVkResult(vkCreateSwapchainKHR(device->getVkDevice(), &createInfo, nullptr, &result));

  // Remove the old-swapchain.
  vkDestroySwapchainKHR(device->getVkDevice(), oldSwapchain, nullptr);

  return result;
}

[[nodiscard]] auto createSurfaceImageView(const Device* device, VkImage surfaceImage)
    -> VkImageView {
  assert(device);

  VkImageViewCreateInfo createInfo           = {};
  createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image                           = surfaceImage;
  createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format                          = device->getVkSurfaceFormat().format;
  createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  createInfo.subresourceRange.baseMipLevel   = 0U;
  createInfo.subresourceRange.levelCount     = 1U;
  createInfo.subresourceRange.baseArrayLayer = 0U;
  createInfo.subresourceRange.layerCount     = 1U;

  VkImageView result;
  checkVkResult(vkCreateImageView(device->getVkDevice(), &createInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createSurfaceFramebuffer(
    const Device* device, VkRenderPass vkRenderPass, VkImageView imageView, VkExtent2D extent) {
  assert(device);

  std::array<VkImageView, 1> attachments = {
      imageView,
  };
  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass              = vkRenderPass;
  framebufferInfo.attachmentCount         = 1;
  framebufferInfo.pAttachments            = attachments.data();
  framebufferInfo.width                   = extent.width;
  framebufferInfo.height                  = extent.height;
  framebufferInfo.layers                  = 1;

  VkFramebuffer result;
  checkVkResult(vkCreateFramebuffer(device->getVkDevice(), &framebufferInfo, nullptr, &result));
  return result;
}

} // namespace

Swapchain::Swapchain(log::Logger* logger, const Device* device, VSyncMode vSync) :
    m_logger{logger}, m_device{device}, m_vSync{vSync}, m_vkSwapchain{nullptr} {
  if (!m_device) {
    throw std::invalid_argument{"Device cannot be null"};
  }
}

Swapchain::~Swapchain() {
  try {
    // Wait for all rendering to be done.
    checkVkResult(vkDeviceWaitIdle(m_device->getVkDevice()));

    for (const auto& vkImageView : m_vkImageViews) {
      vkDestroyImageView(m_device->getVkDevice(), vkImageView, nullptr);
    }
    for (const auto& vkFramebuffer : m_vkFramebuffers) {
      vkDestroyFramebuffer(m_device->getVkDevice(), vkFramebuffer, nullptr);
    }
    if (m_vkSwapchain) {
      vkDestroySwapchainKHR(m_device->getVkDevice(), m_vkSwapchain, nullptr);
    }
    LOG_D(m_logger, "Vulkan swapchain destroyed");

  } catch (...) {
    LOG_E(m_logger, "Failed to cleanup vulkan swapchain");
  }
}

auto Swapchain::getVkFramebuffer(uint32_t imageIndex) const -> const VkFramebuffer& {
  if (imageIndex >= m_vkFramebuffers.size()) {
    throw std::invalid_argument{"Image-index is out of range"};
  }
  return m_vkFramebuffers[imageIndex];
}

auto Swapchain::acquireImage(VkRenderPass vkRenderPass, VkSemaphore imgAvailable, bool forceReinit)
    -> std::optional<uint32_t> {

  if (!m_vkSwapchain || m_swapchainOutOfDate || forceReinit) {
    // Wait for all rendering to be done.
    // TODO(bastian): This is a very rough way of synchronizing, investigate potentially keeping
    // both swapchains alive during swapchain recreation.
    vkDeviceWaitIdle(m_device->getVkDevice());

    if (!initSwapchain(vkRenderPass)) {
      // We failed to create a swapchain (for example because the window is mimized).
      return std::nullopt;
    }
  }

  // Fail to acquire an image if the extent is 0 (can happen if the window is minized).
  if (m_extent.width == 0 || m_extent.height == 0) {
    return std::nullopt;
  }

  uint32_t imgIndex;
  const auto result = vkAcquireNextImageKHR(
      m_device->getVkDevice(), m_vkSwapchain, UINT64_MAX, imgAvailable, nullptr, &imgIndex);

  switch (result) {
  case VK_ERROR_OUT_OF_DATE_KHR:
    LOG_D(m_logger, "Out-of-date swapchain detected during acquire");
    // Attempt again to acquire an image (will recreate the swapchain).
    return acquireImage(vkRenderPass, imgAvailable, true);
  case VK_SUBOPTIMAL_KHR:
    // Set a flag to recreate the swapchain on the next frame.
    m_swapchainOutOfDate = true;
    LOG_D(m_logger, "Sub-optimal swapchain detected during acquire");
    break;
  default:
    checkVkResult(result);
  }
  return imgIndex;
}

auto Swapchain::presentImage(VkSemaphore imgReady, uint32_t imageIndex) -> bool {

  std::array<VkSemaphore, 1> semaphores = {
      imgReady,
  };
  std::array<VkSwapchainKHR, 1> swapChains = {
      m_vkSwapchain,
  };

  VkPresentInfoKHR presentInfo   = {};
  presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = semaphores.size();
  presentInfo.pWaitSemaphores    = semaphores.data();
  presentInfo.swapchainCount     = swapChains.size();
  presentInfo.pSwapchains        = swapChains.data();
  presentInfo.pImageIndices      = &imageIndex;

  const auto result = vkQueuePresentKHR(m_device->getVkPresentQueue(), &presentInfo);

  switch (result) {
  case VK_SUBOPTIMAL_KHR:
    m_swapchainOutOfDate = true;
    LOG_D(m_logger, "Sub-optimal swapchain detected during present");
    return true; // Presenting will still succeed.
  case VK_ERROR_OUT_OF_DATE_KHR:
    m_swapchainOutOfDate = true;
    LOG_D(m_logger, "Out-of-date swapchain detected during present");
    return false; // Presenting will fail.
  default:
    checkVkResult(result);
    return true;
  }
}

auto Swapchain::initSwapchain(VkRenderPass vkRenderPass) -> bool {

  const auto capabilities = m_device->queryVkSurfaceCapabilities();
  m_imgCount              = getImageCount(capabilities);
  m_extent                = getSwapExtent(capabilities);
  const auto transform    = capabilities.currentTransform;
  const auto presentMode  = getPresentMode(m_device, m_vSync);

  // Size 0 can happen when minimizing a window, in that case we cannot create a swapchain.
  if (m_extent.height == 0U || m_extent.width == 0U) {
    return false;
  }

  // Delete any imageViews that belonged to the previous swapchain.
  for (const auto& vkImageView : m_vkImageViews) {
    vkDestroyImageView(m_device->getVkDevice(), vkImageView, nullptr);
  }
  m_vkImageViews.clear();

  // Delete any frameBuffers that belonged to the previous swapchain.
  for (const auto& vkFramebuffer : m_vkFramebuffers) {
    vkDestroyFramebuffer(m_device->getVkDevice(), vkFramebuffer, nullptr);
  }
  m_vkFramebuffers.clear();

  // Create the swapchain and retrieve the swapchain images.
  m_vkSwapchain =
      createVkSwapchain(m_device, m_vkSwapchain, m_imgCount, m_extent, transform, presentMode);
  m_vkImages = getSwapchainVkImages(m_device->getVkDevice(), m_vkSwapchain);

  // Create an imageview for every swapchain image.
  for (const auto& vkImage : m_vkImages) {
    m_vkImageViews.push_back(createSurfaceImageView(m_device, vkImage));
  }

  // Create a framebuffer for every swapchain image-view.
  for (const auto& vkImageView : m_vkImageViews) {
    m_vkFramebuffers.push_back(
        createSurfaceFramebuffer(m_device, vkRenderPass, vkImageView, m_extent));
  }

  LOG_D(
      m_logger,
      "Vulkan swapchain created",
      {"vSync", getName(m_vSync)},
      {"presentMode", getVkPresentModeString(presentMode)},
      {"imageCount", m_imgCount},
      {"size", m_extent.width, m_extent.height});

  m_swapchainOutOfDate = false;
  return true;
}

} // namespace tria::gfx::internal

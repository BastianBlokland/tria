#include "swapchain.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getSwapchainImageCount(const VkSurfaceCapabilitiesKHR& capabilities) noexcept
    -> uint32_t {
  // We want to use 2 images (one on screen and one being rendered to). But we also have to repsect
  // the surface capabilities.
  auto desired = std::max(capabilities.minImageCount, 2U);
  if (capabilities.maxImageCount != 0U && capabilities.maxImageCount < desired) {
    return capabilities.maxImageCount;
  }
  return desired;
}

[[nodiscard]] auto getSwapchainImageSize(const VkSurfaceCapabilitiesKHR& capabilities) noexcept
    -> SwapchainSize {
  if (capabilities.currentExtent.width != UINT32_MAX &&
      capabilities.currentExtent.height != UINT32_MAX) {
    return {capabilities.currentExtent.width, capabilities.currentExtent.height};
  } else {
    // Note: in this case the surface does not have a preference what size the swapchain should be.
    // Probably in this case we should use the size of the window, however i have not seen this
    // happen on real platforms.
    return {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
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
  // Prefer fifo_relaxed to reduce stuttering in case of late frames.
  for (const auto& mode : modes) {
    if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
      return mode;
    }
  }
  // Fifo is guaranteed to be available.
  return VK_PRESENT_MODE_FIFO_KHR;
}

[[nodiscard]] auto createVkSwapchain(
    const Device* device,
    VkSwapchainKHR oldSwapchain,
    uint32_t imgCount,
    SwapchainSize size,
    VkSurfaceTransformFlagBitsKHR transformFlags,
    VkPresentModeKHR presentMode) -> VkSwapchainKHR {
  assert(device);

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface                  = device->getVkSurface();
  createInfo.minImageCount            = imgCount;
  createInfo.imageFormat              = device->getVkSurfaceFormat().format;
  createInfo.imageColorSpace          = device->getVkSurfaceFormat().colorSpace;
  createInfo.imageExtent.width        = size.x();
  createInfo.imageExtent.height       = size.y();
  createInfo.imageArrayLayers         = 1U;
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

} // namespace

Swapchain::Swapchain(log::Logger* logger, const Device* device, VSyncMode vSync) :
    m_logger{logger}, m_device{device}, m_vSync{vSync}, m_vkSwapchain{nullptr}, m_version{0U} {
  if (!m_device) {
    throw std::invalid_argument{"Device cannot be null"};
  }
}

Swapchain::~Swapchain() {
  try {
    // Wait for all rendering to be done.
    checkVkResult(vkDeviceWaitIdle(m_device->getVkDevice()));

    m_images.clear();
    if (m_vkSwapchain) {
      vkDestroySwapchainKHR(m_device->getVkDevice(), m_vkSwapchain, nullptr);
    }
    LOG_D(m_logger, "Vulkan swapchain destroyed");

  } catch (...) {
    LOG_E(m_logger, "Failed to cleanup vulkan swapchain");
  }
}

auto Swapchain::getImage(SwapchainIdx idx) const -> const Image& {
  if (idx >= m_images.size()) {
    throw std::invalid_argument{"Image-index is out of range"};
  }
  return m_images[idx];
}

auto Swapchain::acquireImage(VkSemaphore imgAvailable, bool forceReinit)
    -> std::optional<SwapchainIdx> {

  if (!m_vkSwapchain || m_swapchainOutOfDate || forceReinit) {
    // Wait for all rendering to be done.
    // TODO(bastian): This is a very rough way of synchronizing, investigate potentially keeping
    // both swapchains alive during swapchain recreation.
    vkDeviceWaitIdle(m_device->getVkDevice());

    if (!initSwapchain()) {
      // We failed to create a swapchain (for example because the window is mimized).
      return std::nullopt;
    }
  }

  // Fail to acquire an image if the size is 0 (can happen if the window is minized).
  if (m_size.x() == 0 || m_size.y() == 0) {
    return std::nullopt;
  }

  uint32_t imgIndex;
  const auto result = vkAcquireNextImageKHR(
      m_device->getVkDevice(), m_vkSwapchain, UINT64_MAX, imgAvailable, nullptr, &imgIndex);

  switch (result) {
  case VK_ERROR_OUT_OF_DATE_KHR:
    LOG_D(m_logger, "Out-of-date swapchain detected during acquire");
    // Attempt again to acquire an image (will recreate the swapchain).
    return acquireImage(imgAvailable, true);
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

auto Swapchain::initSwapchain() -> bool {

  const auto capabilities = m_device->queryVkSurfaceCapabilities();
  const auto imgCount     = getSwapchainImageCount(capabilities);
  m_size                  = getSwapchainImageSize(capabilities);
  const auto transform    = capabilities.currentTransform;
  const auto presentMode  = getPresentMode(m_device, m_vSync);

  // Size 0 can happen when minimizing a window, in that case we cannot create a swapchain.
  if (m_size.x() == 0U || m_size.y() == 0U) {
    return false;
  }

  // Delete any images that belonged to the previous swapchain.
  m_images.clear();

  // Create the swapchain and retrieve the swapchain images.
  m_vkSwapchain =
      createVkSwapchain(m_device, m_vkSwapchain, imgCount, m_size, transform, presentMode);
  const auto vkImages = getSwapchainVkImages(m_device->getVkDevice(), m_vkSwapchain);

  DBG_SWAPCHAIN_NAME(m_device, m_vkSwapchain, "swapchain");

  // Create images.
  for (auto i = 0U; i != vkImages.size(); ++i) {
    auto img = Image{
        m_device, vkImages[i], m_size, m_device->getVkSurfaceFormat().format, ImageType::Swapchain};

    DBG_IMG_NAME(m_device, img.getVkImage(), "swapchain_" + std::to_string(i));
    DBG_IMGVIEW_NAME(m_device, img.getVkImageView(), "swapchain_" + std::to_string(i));

    m_images.push_back(std::move(img));
  }

  LOG_D(
      m_logger,
      "Vulkan swapchain created",
      {"vSync", getName(m_vSync)},
      {"presentMode", getVkPresentModeString(presentMode)},
      {"imageCount", m_images.size()},
      {"size", m_size});

  m_swapchainOutOfDate = false;
  ++m_version;
  return true;
}

} // namespace tria::gfx::internal

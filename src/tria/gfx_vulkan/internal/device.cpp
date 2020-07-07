#include "device.hpp"
#include "tria/gfx/err/driver_err.hpp"
#include "utils.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <map>
#include <optional>
#include <set>

namespace tria::gfx::internal {

namespace {

constexpr std::array<const char*, 1> requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

[[nodiscard]] auto getVkPhysicalDevices(VkInstance vkInstance) -> std::vector<VkPhysicalDevice> {
  uint32_t count = 0;
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &count, nullptr));
  auto result = std::vector<VkPhysicalDevice>{count};
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &count, result.data()));
  return result;
}

[[nodiscard]] auto getVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice)
    -> std::vector<VkQueueFamilyProperties> {
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
  auto result = std::vector<VkQueueFamilyProperties>{count};
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, result.data());
  return result;
}

[[nodiscard]] auto getVkDeviceExtensions(VkPhysicalDevice vkPhysicalDevice)
    -> std::vector<VkExtensionProperties> {
  uint32_t count = 0;
  checkVkResult(vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, nullptr));
  auto result = std::vector<VkExtensionProperties>{count};
  checkVkResult(
      vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, result.data()));
  return result;
}

[[nodiscard]] auto getVkSurfaceFormats(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkSurfaceFormatKHR> {
  uint32_t count = 0;
  checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, nullptr));
  auto result = std::vector<VkSurfaceFormatKHR>{count};
  checkVkResult(
      vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, result.data()));
  return result;
}

[[nodiscard]] auto getVkPresentModes(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkPresentModeKHR> {
  uint32_t count = 0;
  checkVkResult(
      vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &count, nullptr));
  auto result = std::vector<VkPresentModeKHR>{count};
  checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vkPhysicalDevice, vkSurface, &count, result.data()));
  return result;
}

[[nodiscard]] auto getSwapchainVkImages(VkDevice vkDevice, VkSwapchainKHR vkSwapchain)
    -> std::vector<VkImage> {
  uint32_t count = 0;
  checkVkResult(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &count, nullptr));
  auto result = std::vector<VkImage>{count};
  checkVkResult(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &count, result.data()));
  return result;
}

[[nodiscard]] auto pickGraphicsQueueIdx(const std::vector<VkQueueFamilyProperties>& queues) noexcept
    -> std::optional<uint32_t> {
  for (auto i = 0U; i != queues.size(); ++i) {
    if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return i;
    }
  }
  return std::nullopt;
}

[[nodiscard]] auto pickPresentQueueIdx(
    const std::vector<VkQueueFamilyProperties>& queues,
    VkPhysicalDevice vkPhysicalDevice,
    VkSurfaceKHR vkSurface) -> std::optional<uint32_t> {

  for (auto i = 0U; i != queues.size(); ++i) {
    VkBool32 presentSupport = false;
    checkVkResult(
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport));
    if (presentSupport) {
      return i;
    }
  }
  return std::nullopt;
}

[[nodiscard]] auto
pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) noexcept
    -> std::optional<VkSurfaceFormatKHR> {

  // Prefer srgb.
  for (const auto& format : availableFormats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  // If thats not availble then pick whatever is available.
  // Note: In the future we could consider add some scoring algorithm here.
  return availableFormats.empty() ? std::nullopt : std::optional(*availableFormats.begin());
}

[[nodiscard]] auto pickPresentMode(const std::vector<VkPresentModeKHR>& availableModes) noexcept
    -> VkPresentModeKHR {

  // Prefer mailbox.
  for (const auto& mode : availableModes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  // If thats not availble then fall back to fifo (which is guaranteed to be available).
  return VK_PRESENT_MODE_FIFO_KHR;
}

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

[[nodiscard]] auto getSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities, uint16_t width, uint16_t height) noexcept
    -> VkExtent2D {
  // Note: If current extent is not equal to uint32_max then we have to respect that, otherwise
  // we get to choose ourselves.
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    auto extent = VkExtent2D{width, height};
    // We have to respect the min and max image extent capabilities of the device.
    extent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, extent.height));
    return extent;
  }
}

[[nodiscard]] auto createVkDevice(VkPhysicalDevice physicalDevice, std::set<uint32_t> queueFamilies)
    -> VkDevice {

  // Set of required device features, everything false at the moment.
  VkPhysicalDeviceFeatures deviceFeatures = {};

  // Queues to create on the device.
  auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo>{};
  queueCreateInfos.reserve(queueFamilies.size());
  for (auto queueFamily : queueFamilies) {
    std::array<float, 1> queuePriorities    = {1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex        = queueFamily;
    queueCreateInfo.queueCount              = 1;
    queueCreateInfo.pQueuePriorities        = queuePriorities.data();
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkDeviceCreateInfo createInfo      = {};
  createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.queueCreateInfoCount    = queueCreateInfos.size();
  createInfo.enabledExtensionCount   = requiredDeviceExtensions.size();
  createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
  createInfo.pEnabledFeatures        = &deviceFeatures;

  VkDevice result;
  checkVkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

Device::Device(log::Logger* logger, VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) :
    m_logger{logger},
    m_vkPhysicalDevice{vkPhysicalDevice},
    m_vkSurface{vkSurface},
    m_vkSwapchain{nullptr} {

  vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_properties);
  vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &m_features);
  checkVkResult(
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &m_capabilities));

  const auto queueFamilies    = getVkQueueFamilies(vkPhysicalDevice);
  const auto foundGfxQueueIdx = pickGraphicsQueueIdx(queueFamilies);
  if (foundGfxQueueIdx) {
    m_graphicsQueueIdx = *foundGfxQueueIdx;
  } else {
    throw err::DriverErr{"Selected vulkan device is missing a graphics queue"};
  }
  const auto foundPresentQueueIdx = pickPresentQueueIdx(queueFamilies, vkPhysicalDevice, vkSurface);
  if (foundPresentQueueIdx) {
    m_presentQueueIdx = *foundPresentQueueIdx;
  } else {
    throw err::DriverErr{"Selected vulkan device is missing a presentation queue"};
  }

  const auto surfaceFormats     = getVkSurfaceFormats(vkPhysicalDevice, vkSurface);
  const auto foundSurfaceFormat = pickSurfaceFormat(surfaceFormats);
  if (foundSurfaceFormat) {
    m_surfaceFormat = *foundSurfaceFormat;
  } else {
    throw err::DriverErr{"Selected vulkan device is missing a suitable surface format"};
  }

  const auto presentModes = getVkPresentModes(vkPhysicalDevice, vkSurface);
  m_presentMode           = pickPresentMode(presentModes);

  m_vkDevice = createVkDevice(m_vkPhysicalDevice, {*foundGfxQueueIdx, *foundPresentQueueIdx});
  vkGetDeviceQueue(m_vkDevice, *foundGfxQueueIdx, 0U, &m_graphicsQueue);
  vkGetDeviceQueue(m_vkDevice, *foundPresentQueueIdx, 0U, &m_presentQueue);

  LOG_I(
      m_logger,
      "Device created",
      {"deviceId", m_properties.deviceID},
      {"deviceName", m_properties.deviceName},
      {"graphicsQueueIdx", m_graphicsQueueIdx},
      {"presentQueueIdx", m_presentQueueIdx},
      {"surfaceFormat", getVkFormatString(m_surfaceFormat.format)},
      {"surfaceColorSpace", getVkColorSpaceString(m_surfaceFormat.colorSpace)},
      {"presentMode", getVkPresentModeString(m_presentMode)});
}

Device::~Device() {
  for (const auto& vkImageView : m_swapchainVkImageViews) {
    vkDestroyImageView(m_vkDevice, vkImageView, nullptr);
  }
  if (m_vkSwapchain != nullptr) {
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);
  }
  vkDestroyDevice(m_vkDevice, nullptr);
}

auto Device::initSwapchain(uint16_t width, uint16_t height) -> void {
  // Delete any imageView that belonged to the previous swapchain.
  for (const auto& vkImageView : m_swapchainVkImageViews) {
    vkDestroyImageView(m_vkDevice, vkImageView, nullptr);
  }
  m_swapchainVkImageViews.clear();

  auto extent   = getSwapExtent(m_capabilities, width, height);
  auto imgCount = getImageCount(m_capabilities);

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface          = m_vkSurface;
  createInfo.minImageCount    = imgCount;
  createInfo.imageFormat      = m_surfaceFormat.format;
  createInfo.imageColorSpace  = m_surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array<uint32_t, 2> queueFamilyIndices = {m_graphicsQueueIdx, m_presentQueueIdx};
  if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    // Note: For compatiblity we support devices where the graphics and present queues are not the
    // same, however this is not a common case. If it becomes a more common case we should probably
    // setup explicit ownership transfers instead of the concurrent mode for higher performance.
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
  }

  createInfo.preTransform   = m_capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode    = m_presentMode;
  createInfo.clipped        = VK_TRUE;
  createInfo.oldSwapchain   = m_vkSwapchain;

  // Create the swapchain and retrieve the swapchain images.
  checkVkResult(vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_vkSwapchain));
  m_swapchainVkImages = getSwapchainVkImages(m_vkDevice, m_vkSwapchain);
  m_swapchainSize     = extent;

  // Create an imageview for every swapchain image.
  for (const auto& vkImage : m_swapchainVkImages) {
    VkImageViewCreateInfo createInfo           = {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = vkImage;
    createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format                          = m_surfaceFormat.format;
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;
    VkImageView result;
    checkVkResult(vkCreateImageView(m_vkDevice, &createInfo, nullptr, &result));
    m_swapchainVkImageViews.push_back(result);
  }

  LOG_D(
      m_logger,
      "Swapchain created",
      {"imageCount", imgCount},
      {"sharedGfxPresentQueue", m_graphicsQueueIdx == m_presentQueueIdx},
      {"width", extent.width},
      {"height", extent.height});
}

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance, VkSurfaceKHR vkSurface)
    -> DevicePtr {

  // List of devices sorted by score.
  auto devices = std::multimap<unsigned int, VkPhysicalDevice>{};

  for (const auto& vkPhysicalDevice : getVkPhysicalDevices(vkInstance)) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);

    auto availableExts = getVkDeviceExtensions(vkPhysicalDevice);

    auto deviceIsSuitable = true;
    // Check if the device supports all the extensions we want.
    for (const auto& required : requiredDeviceExtensions) {
      if (!std::any_of(
              availableExts.begin(),
              availableExts.end(),
              [required](const VkExtensionProperties& e) {
                return std::strcmp(required, e.extensionName) == 0;
              })) {
        deviceIsSuitable = false;
      }
    }

    // Just prefer any discrete gpu, in the future we could do smarter selection here.
    auto score = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1U : 2U;

    LOG_D(
        logger,
        "Found physical device",
        {"deviceId", properties.deviceID},
        {"deviceName", properties.deviceName},
        {"deviceType", getVkDeviceTypeString(properties.deviceType)},
        {"vendorId", properties.vendorID},
        {"vendorName", getVkVendorString(properties.vendorID)},
        {"suitable", deviceIsSuitable},
        {"score", score});

    if (deviceIsSuitable) {
      devices.insert({score, vkPhysicalDevice});
    }
  }

  // Select the device with the highest score.
  return devices.empty() ? nullptr
                         : std::make_unique<Device>(logger, devices.rbegin()->second, vkSurface);
}

} // namespace tria::gfx::internal

#include "device.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "tria/pal/native.hpp"
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

[[nodiscard]] auto createVkSurfaceKhr(VkInstance vkInstance, const pal::Window* window)
    -> VkSurfaceKHR {
  VkSurfaceKHR result;
#if defined(TRIA_LINUX_XCB)
  VkXcbSurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.connection                = pal::getLinuxXcbConnection(*window);
  createInfo.window                    = pal::getLinuxXcbWindow(*window);
  checkVkResult(vkCreateXcbSurfaceKHR(vkInstance, &createInfo, nullptr, &result));
#elif defined(TRIA_WIN32)
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance                   = pal::getWin32HInstance(*window);
  createInfo.hwnd                        = pal::getWin32HWnd(*window);
  checkVkResult(vkCreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, &result));
#else
  static_assert("false", "Unsupported platform");
#endif
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
    VkBool32 presentSupport;
    checkVkResult(
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport));
    if (presentSupport) {
      return i;
    }
  }
  return std::nullopt;
}

[[nodiscard]] auto
pickSurfaceFormat(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) noexcept
    -> std::optional<VkSurfaceFormatKHR> {

  const auto availableFormats = getVkSurfaceFormats(vkPhysicalDevice, vkSurface);

  // Prefer SRGB.
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

[[nodiscard]] auto
createVkDevice(VkPhysicalDevice vkPhysicalDevice, std::set<uint32_t> queueFamilies) -> VkDevice {

  // Set of required device features, nothing enabled at the moment.
  VkPhysicalDeviceFeatures deviceFeatures = {};

  // Queues to create on the device.
  auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo>{};
  queueCreateInfos.reserve(queueFamilies.size());
  for (auto queueFamily : queueFamilies) {
    std::array<float, 1> queuePriorities    = {1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex        = queueFamily;
    queueCreateInfo.queueCount              = queuePriorities.size();
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
  checkVkResult(vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkCommandPool(VkDevice vkDevice, uint32_t queueFamily) -> VkCommandPool {
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex        = queueFamily;
  poolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool result;
  checkVkResult(vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &result));
  return result;
}

} // namespace

Device::Device(
    log::Logger* logger,
    VkInstance vkInstance,
    VkPhysicalDevice vkPhysicalDevice,
    const pal::Window* window) :
    m_logger{logger}, m_vkInstance{vkInstance}, m_vkPhysicalDevice{vkPhysicalDevice} {

  // Query supported properties and supported features.
  vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_properties);
  vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &m_features);
  vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &m_memProperties);

  // Create a vulkan surface targetting the given window.
  m_vkSurface                   = createVkSurfaceKhr(vkInstance, window);
  const auto foundSurfaceFormat = pickSurfaceFormat(vkPhysicalDevice, m_vkSurface);
  if (foundSurfaceFormat) {
    m_surfaceFormat = *foundSurfaceFormat;
  } else {
    throw err::GfxErr{"Selected vulkan device is missing a suitable surface format"};
  }

  // Pick a queue family for the graphics and present queue to create on the device.
  const auto queueFamilies    = getVkQueueFamilies(vkPhysicalDevice);
  const auto foundGfxQueueIdx = pickGraphicsQueueIdx(queueFamilies);
  if (foundGfxQueueIdx) {
    m_graphicsQueueIdx = *foundGfxQueueIdx;
  } else {
    throw err::GfxErr{"Selected vulkan device is missing a graphics queue"};
  }
  const auto foundPresentQueueIdx =
      pickPresentQueueIdx(queueFamilies, vkPhysicalDevice, m_vkSurface);
  if (foundPresentQueueIdx) {
    m_presentQueueIdx = *foundPresentQueueIdx;
  } else {
    throw err::GfxErr{"Selected vulkan device is missing a presentation queue"};
  }

  // Create a logical device and retrieve the created queues.
  m_vkDevice = createVkDevice(m_vkPhysicalDevice, {*foundGfxQueueIdx, *foundPresentQueueIdx});
  vkGetDeviceQueue(m_vkDevice, *foundGfxQueueIdx, 0U, &m_graphicsQueue);
  vkGetDeviceQueue(m_vkDevice, *foundPresentQueueIdx, 0U, &m_presentQueue);

  // Create a command-pool to create graphics command-buffers from.
  m_graphicsVkCommandPool = createVkCommandPool(m_vkDevice, m_graphicsQueueIdx);

  // Create a global memory pool to allocate from.
  m_memory =
      std::make_unique<MemoryPool>(m_logger, m_vkDevice, m_memProperties, m_properties.limits);

  LOG_I(
      m_logger,
      "Vulkan device created",
      {"deviceId", m_properties.deviceID},
      {"deviceName", m_properties.deviceName},
      {"graphicsQueueIdx", m_graphicsQueueIdx},
      {"presentQueueIdx", m_presentQueueIdx},
      {"surfaceFormat", getVkFormatString(m_surfaceFormat.format)},
      {"surfaceColorSpace", getVkColorSpaceString(m_surfaceFormat.colorSpace)});
}

Device::~Device() {
  // Wait for all rendering to be done before destroying the device.
  vkDeviceWaitIdle(m_vkDevice);

  m_memory = nullptr;
  vkDestroyCommandPool(m_vkDevice, m_graphicsVkCommandPool, nullptr);
  vkDestroyDevice(m_vkDevice, nullptr);
  vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
}

auto Device::queryVkSurfaceCapabilities() const -> VkSurfaceCapabilitiesKHR {
  VkSurfaceCapabilitiesKHR result;
  checkVkResult(
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &result));
  return result;
}

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance, const pal::Window* window)
    -> DeviceUnique {

  // List of devices sorted by score.
  auto devices = std::multimap<unsigned int, VkPhysicalDevice>{};

  for (const auto& vkPhysicalDevice : getVkPhysicalDevices(vkInstance)) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);

    const auto availableExts = getVkDeviceExtensions(vkPhysicalDevice);

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
    const auto score = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1U : 2U;

    LOG_D(
        logger,
        "Found Vulkan physical device",
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
  return devices.empty()
      ? nullptr
      : std::make_unique<Device>(logger, vkInstance, devices.rbegin()->second, window);
}

} // namespace tria::gfx::internal

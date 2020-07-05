#include "device.hpp"
#include "tria/gfx/err/driver_err.hpp"
#include "utils.hpp"
#include <array>
#include <map>
#include <optional>

namespace tria::gfx::internal {

namespace {

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

[[nodiscard]] auto getGraphicsQueueIdx(const std::vector<VkQueueFamilyProperties>& queues)
    -> std::optional<uint32_t> {
  for (auto i = 0U; i != queues.size(); ++i) {
    if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return i;
    }
  }
  return std::nullopt;
}

[[maybe_unused]] [[nodiscard]] auto getVKDeviceTypeString(VkPhysicalDeviceType type) noexcept
    -> const char* {
  switch (type) {
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return "integrated";
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return "discrete";
  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    return "virtual";
  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return "cpu";
  default:
    return "other";
  }
}

[[maybe_unused]] [[nodiscard]] auto getVendorString(uint32_t vendorId) noexcept -> const char* {
  switch (vendorId) {
  case 0x1002:
    return "AMD";
  case 0x1010:
    return "ImgTec";
  case 0x10DE:
    return "NVIDIA";
  case 0x13B5:
    return "ARM";
  case 0x5143:
    return "Qualcomm";
  case 0x8086:
    return "INTEL";
  default:
    return "other";
  }
}

[[nodiscard]] auto createVkDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueue)
    -> VkDevice {

  // Set of required device features, everything false at the moment.
  VkPhysicalDeviceFeatures deviceFeatures = {};

  // Queues to create on the device.
  // At the moment only a graphics queue.
  std::array<float, 1> queuePriority      = {1.0f};
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex        = graphicsQueue;
  queueCreateInfo.queueCount              = 1;
  queueCreateInfo.pQueuePriorities        = queuePriority.data();

  VkDeviceCreateInfo createInfo   = {};
  createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos    = &queueCreateInfo;
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures     = &deviceFeatures;

  VkDevice result;
  checkVkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

Device::Device(log::Logger* logger, VkPhysicalDevice vkPhysicalDevice) :
    m_logger{logger}, m_vkPhysicalDevice{vkPhysicalDevice} {

  // Get device properties and features from vulkan.
  vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_properties);
  vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &m_features);

  auto queueFamilies    = getVkQueueFamilies(vkPhysicalDevice);
  auto foundGfxQueueIdx = getGraphicsQueueIdx(queueFamilies);
  if (!foundGfxQueueIdx) {
    throw err::DriverErr{"Selected device is missing a graphics queue"};
  }

  m_vkDevice = createVkDevice(m_vkPhysicalDevice, *foundGfxQueueIdx);
  vkGetDeviceQueue(m_vkDevice, *foundGfxQueueIdx, 0U, &m_graphicsQueue);

  LOG_I(
      m_logger,
      "Device created",
      {"deviceId", m_properties.deviceID},
      {"deviceName", m_properties.deviceName},
      {"graphicsQueueIdx", *foundGfxQueueIdx});
}

Device::~Device() { vkDestroyDevice(m_vkDevice, nullptr); }

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance) -> DevicePtr {

  // List of devices sorted by score.
  auto devices = std::multimap<unsigned int, VkPhysicalDevice>{};

  for (const auto& vkPhysicalDevice : getVkPhysicalDevices(vkInstance)) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);

    // Just select any discrete gpu, in the future we could do smarter selection here.
    auto score = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1U : 0U;

    LOG_D(
        logger,
        "Found physical device",
        {"deviceId", properties.deviceID},
        {"deviceName", properties.deviceName},
        {"deviceType", getVKDeviceTypeString(properties.deviceType)},
        {"vendorId", properties.vendorID},
        {"vendorName", getVendorString(properties.vendorID)},
        {"score", score});

    devices.insert({score, vkPhysicalDevice});
  }

  // Select the device with the highest score.
  return devices.empty() ? nullptr : std::make_unique<Device>(logger, devices.rbegin()->second);
}

} // namespace tria::gfx::internal

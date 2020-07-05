#include "device.hpp"
#include "utils.hpp"
#include <map>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getVkPhysicalDevices(VkInstance vkInstance) -> std::vector<VkPhysicalDevice> {
  uint32_t layerCount = 0;
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &layerCount, nullptr));
  auto result = std::vector<VkPhysicalDevice>{layerCount};
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &layerCount, result.data()));
  return result;
}

[[maybe_unused]] [[nodiscard]] auto getVKDeviceTypeString(VkPhysicalDeviceType type) -> const
    char* {
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

[[maybe_unused]] [[nodiscard]] auto getVendorString(uint32_t vendorId) -> const char* {
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

} // namespace

Device::Device(log::Logger* logger, VkInstance /*unused*/, VkPhysicalDevice vkPhysicalDevice) :
    m_logger{logger}, m_vkPhysicalDevice{vkPhysicalDevice} {

  // Get device properties and features from vulkan.
  vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_properties);
  vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &m_features);

  LOG_I(
      m_logger,
      "Device created",
      {"deviceId", m_properties.deviceID},
      {"deviceName", m_properties.deviceName});
}

Device::~Device() {}

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
  return devices.empty() ? nullptr
                         : std::make_unique<Device>(logger, vkInstance, devices.rbegin()->second);
}

} // namespace tria::gfx::internal

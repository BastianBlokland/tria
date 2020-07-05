#pragma once
#include "tria/log/api.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device final {
public:
  Device(log::Logger* logger, VkPhysicalDevice vkPhysicalDevice);
  ~Device();

private:
  log::Logger* m_logger;
  VkPhysicalDevice m_vkPhysicalDevice;
  VkPhysicalDeviceProperties m_properties;
  VkPhysicalDeviceFeatures m_features;
  VkDevice m_vkDevice;
  VkQueue m_graphicsQueue;
};

using DevicePtr = std::unique_ptr<Device>;

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance) -> DevicePtr;

} // namespace tria::gfx::internal

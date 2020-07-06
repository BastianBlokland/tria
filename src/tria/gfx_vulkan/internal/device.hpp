#pragma once
#include "tria/log/api.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device final {
public:
  Device(log::Logger* logger, VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);
  ~Device();

private:
  log::Logger* m_logger;
  VkPhysicalDevice m_vkPhysicalDevice;
  VkPhysicalDeviceProperties m_properties;
  VkPhysicalDeviceFeatures m_features;
  VkSurfaceCapabilitiesKHR m_capabilities;
  VkDevice m_vkDevice;
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;
  VkSurfaceFormatKHR m_surfaceFormat;
  VkPresentModeKHR m_presentMode;
};

using DevicePtr = std::unique_ptr<Device>;

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance, VkSurfaceKHR vkSurface)
    -> DevicePtr;

} // namespace tria::gfx::internal

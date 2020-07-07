#pragma once
#include "tria/log/api.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device final {
public:
  Device(log::Logger* logger, VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);
  ~Device();

  auto initSwapchain(uint16_t width, uint16_t height) -> void;

private:
  log::Logger* m_logger;
  VkPhysicalDevice m_vkPhysicalDevice;
  VkSurfaceKHR m_vkSurface;
  VkPhysicalDeviceProperties m_properties;
  VkPhysicalDeviceFeatures m_features;
  VkSurfaceCapabilitiesKHR m_capabilities;
  VkDevice m_vkDevice;
  VkQueue m_graphicsQueue;
  uint32_t m_graphicsQueueIdx;
  VkQueue m_presentQueue;
  uint32_t m_presentQueueIdx;
  VkSurfaceFormatKHR m_surfaceFormat;
  VkPresentModeKHR m_presentMode;
  VkSwapchainKHR m_vkSwapchain;
  std::vector<VkImage> m_swapchainVkImages;
  std::vector<VkImageView> m_swapchainVkImageViews;
  VkExtent2D m_swapchainSize;
};

using DevicePtr = std::unique_ptr<Device>;

[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance, VkSurfaceKHR vkSurface)
    -> DevicePtr;

} // namespace tria::gfx::internal

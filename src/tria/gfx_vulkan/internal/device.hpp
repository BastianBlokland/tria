#pragma once
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Abstraction of a graphics device and a surface (window) to render to.
 * Reason why the device is combined with the surface is that you might want / need different device
 * features for different surfaces. So combining them allows us to pick a device best suited for
 * rendering to the given window.
 *
 * Note: Surface-format is treated as something that is constant throughout the application's
 * lifetime, technically there is nothing in the Vulkan spec that enforces this. So if we find a
 * platform where the surface-format can change we need to add some additional handling.
 */
class Device final {
public:
  Device(
      log::Logger* logger,
      VkInstance vkInstance,
      VkPhysicalDevice vkPhysicalDevice,
      const pal::Window* window);
  ~Device();

  [[nodiscard]] auto getVkPhysicalDevice() const noexcept { return m_vkPhysicalDevice; }
  [[nodiscard]] auto getVkDevice() const noexcept { return m_vkDevice; }
  [[nodiscard]] auto getVkSurface() const noexcept { return m_vkSurface; }
  [[nodiscard]] auto getVkSurfaceFormat() const noexcept { return m_surfaceFormat; }

  [[nodiscard]] auto getVkGraphicsQueue() const noexcept { return m_graphicsQueue; }
  [[nodiscard]] auto getVkGraphicsQueueIdx() const noexcept { return m_graphicsQueueIdx; }

  [[nodiscard]] auto getVkPresentQueue() const noexcept { return m_presentQueue; }
  [[nodiscard]] auto getVkPresentQueueIdx() const noexcept { return m_presentQueueIdx; }

  [[nodiscard]] auto getGraphicsVkCommandPool() const noexcept { return m_graphicsVkCommandPool; }

  [[nodiscard]] auto queryVkSurfaceCapabilities() const -> VkSurfaceCapabilitiesKHR;

  [[nodiscard]] auto
  getMemoryType(VkMemoryPropertyFlags properties, uint32_t supportedTypesFilter = ~0U) const
      -> uint32_t;

private:
  log::Logger* m_logger;
  VkInstance m_vkInstance;
  VkPhysicalDevice m_vkPhysicalDevice;
  VkPhysicalDeviceProperties m_properties;
  VkPhysicalDeviceFeatures m_features;
  VkPhysicalDeviceMemoryProperties m_memProperties;

  VkSurfaceKHR m_vkSurface;
  VkSurfaceFormatKHR m_surfaceFormat;

  VkDevice m_vkDevice;
  VkQueue m_graphicsQueue;
  uint32_t m_graphicsQueueIdx;
  VkQueue m_presentQueue;
  uint32_t m_presentQueueIdx;

  VkCommandPool m_graphicsVkCommandPool;
};

using DeviceUnique = std::unique_ptr<Device>;

/* Construct a device that is capable of rendering to the given window.
 * Note: returns null if no suitable device is found.
 */
[[nodiscard]] auto getDevice(log::Logger* logger, VkInstance vkInstance, const pal::Window* window)
    -> DeviceUnique;

} // namespace tria::gfx::internal

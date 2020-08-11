#pragma once
#include "descriptor_manager.hpp"
#include "memory_pool.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeContext;

namespace internal {

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
      const NativeContext* context,
      VkPhysicalDevice vkPhysicalDevice,
      const pal::Window* window);
  Device(const Device& rhs)     = delete;
  Device(Device&& rhs) noexcept = delete;
  ~Device();

  auto operator=(const Device& rhs) -> Device& = delete;
  auto operator=(Device&& rhs) noexcept -> Device& = delete;

  [[nodiscard]] auto getContext() const noexcept { return m_context; }
  [[nodiscard]] auto getVkPhysicalDevice() const noexcept { return m_vkPhysicalDevice; }
  [[nodiscard]] auto getVkDevice() const noexcept { return m_vkDevice; }
  [[nodiscard]] auto getVkSurface() const noexcept { return m_vkSurface; }
  [[nodiscard]] auto getVkSurfaceFormat() const noexcept { return m_surfaceFormat; }
  [[nodiscard]] auto getLimits() const noexcept -> const VkPhysicalDeviceLimits& {
    return m_properties.limits;
  }
  [[nodiscard]] auto getFeatures() const noexcept -> const VkPhysicalDeviceFeatures& {
    return m_features;
  }
  [[nodiscard]] auto getMemProperties() const noexcept -> const VkPhysicalDeviceMemoryProperties& {
    return m_memProperties;
  }

  [[nodiscard]] auto getVkGraphicsQueue() const noexcept { return m_graphicsQueue; }
  [[nodiscard]] auto getVkGraphicsQueueIdx() const noexcept { return m_graphicsQueueIdx; }

  [[nodiscard]] auto getVkPresentQueue() const noexcept { return m_presentQueue; }
  [[nodiscard]] auto getVkPresentQueueIdx() const noexcept { return m_presentQueueIdx; }

  [[nodiscard]] auto getGraphicsVkCommandPool() const noexcept { return m_graphicsVkCommandPool; }

  [[nodiscard]] auto getMemory() noexcept -> MemoryPool& { return *m_memory; }
  [[nodiscard]] auto getDescManager() noexcept -> DescriptorManager& { return *m_descManager; }

  [[nodiscard]] auto queryVkSurfaceCapabilities() const -> VkSurfaceCapabilitiesKHR;

  auto setDebugName(VkObjectType vkType, uint64_t vkHandle, std::string_view name) const noexcept
      -> void;

private:
  log::Logger* m_logger;
  const NativeContext* m_context;
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

  internal::MemoryPoolUnique m_memory;
  internal::DescriptorManagerUnique m_descManager;
};

using DeviceUnique = std::unique_ptr<Device>;

/* Construct a device that is capable of rendering to the given window.
 * Note: returns null if no suitable device is found.
 */
[[nodiscard]] auto
getDevice(log::Logger* logger, const NativeContext* context, const pal::Window* window)
    -> DeviceUnique;

} // namespace internal

} // namespace tria::gfx

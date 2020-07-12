#pragma once
#include "tria/gfx/err/driver_err.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

[[nodiscard]] auto getVkAvailableInstanceExtensions() -> std::vector<VkExtensionProperties>;

[[nodiscard]] auto getVkAvailableInstanceLayers() -> std::vector<VkLayerProperties>;

[[nodiscard]] auto getVkPhysicalDevices(VkInstance vkInstance) -> std::vector<VkPhysicalDevice>;

[[nodiscard]] auto getVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice)
    -> std::vector<VkQueueFamilyProperties>;

[[nodiscard]] auto getVkDeviceExtensions(VkPhysicalDevice vkPhysicalDevice)
    -> std::vector<VkExtensionProperties>;

[[nodiscard]] auto getVkSurfaceFormats(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkSurfaceFormatKHR>;

[[nodiscard]] auto getVkPresentModes(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkPresentModeKHR>;

[[nodiscard]] auto getSwapchainVkImages(VkDevice vkDevice, VkSwapchainKHR vkSwapchain)
    -> std::vector<VkImage>;

[[nodiscard]] auto createVkSemaphore(VkDevice vkDevice) -> VkSemaphore;

[[nodiscard]] auto createVkFence(VkDevice vkDevice, bool initialState) -> VkFence;

[[nodiscard]] auto getVkErrStr(VkResult errCode) noexcept -> std::string;
[[nodiscard]] auto getVkDeviceTypeString(VkPhysicalDeviceType type) noexcept -> std::string;
[[nodiscard]] auto getVkVendorString(uint32_t vendorId) noexcept -> std::string;
[[nodiscard]] auto getVkFormatString(VkFormat format) noexcept -> std::string;
[[nodiscard]] auto getVkColorSpaceString(VkColorSpaceKHR colorSpace) noexcept -> std::string;
[[nodiscard]] auto getVkPresentModeString(VkPresentModeKHR mode) noexcept -> std::string;

inline auto checkVkResult(VkResult result) -> void {
  if (result != VK_SUCCESS) {
    throw err::DriverErr{getVkErrStr(result)};
  }

// Usefull to test if the code gracefully handles driver errors.
#if defined(TEST_VK_ERR_HANDLE)
  const auto chance = 0.001;
  if (rand() < RAND_MAX * chance) {
    throw err::DriverErr{"test_error"};
  }
#endif
}

} // namespace tria::gfx::internal

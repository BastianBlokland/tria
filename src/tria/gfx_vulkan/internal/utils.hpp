#pragma once
#include "tria/gfx/err/driver_err.hpp"
#include <string>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

template <typename>
constexpr bool falseValue = false;

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

[[nodiscard]] auto getVkErrStr(VkResult errCode) noexcept -> std::string_view;
[[nodiscard]] auto getVkDeviceTypeString(VkPhysicalDeviceType type) noexcept -> std::string_view;
[[nodiscard]] auto getVkVendorString(uint32_t vendorId) noexcept -> std::string_view;
[[nodiscard]] auto getVkColorSpaceString(VkColorSpaceKHR colorSpace) noexcept -> std::string_view;
[[nodiscard]] auto getVkPresentModeString(VkPresentModeKHR mode) noexcept -> std::string_view;
[[nodiscard]] auto getVkSampleCountString(VkSampleCountFlagBits count) noexcept -> std::string_view;

struct VkFormatInfo final {
  std::string_view name;
  uint32_t size;
  uint32_t channelCount;

  VkFormatInfo(std::string_view name, uint32_t size, uint32_t channelCount) :
      name{name}, size{size}, channelCount{channelCount} {}
};

[[nodiscard]] auto getVkFormatInfo(VkFormat format) noexcept -> VkFormatInfo;

[[nodiscard]] inline auto getVkFormatString(VkFormat format) noexcept -> std::string_view {
  return getVkFormatInfo(format).name;
}

[[nodiscard]] inline auto getVkFormatSize(VkFormat format) noexcept -> uint32_t {
  return getVkFormatInfo(format).size;
}

[[nodiscard]] inline auto getVkFormatChannelCount(VkFormat format) noexcept -> uint32_t {
  return getVkFormatInfo(format).channelCount;
}

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

template <typename T>
[[nodiscard]] constexpr auto getVkIndexType() -> VkIndexType {
  if constexpr (std::is_same_v<T, uint16_t>) {
    return VK_INDEX_TYPE_UINT16;
  } else if constexpr (std::is_same_v<T, uint32_t>) {
    return VK_INDEX_TYPE_UINT32;
  } else {
    static_assert(falseValue<T>, "Unsupported index type");
    return {};
  }
}

/* Calculate the amount of padding required to reach the requested alignment.
 */
[[nodiscard]] constexpr auto padToAlignment(uint32_t value, uint32_t alignment) -> uint32_t {
  const auto rem = value % alignment;
  return rem == 0 ? 0 : alignment - rem;
}

} // namespace tria::gfx::internal

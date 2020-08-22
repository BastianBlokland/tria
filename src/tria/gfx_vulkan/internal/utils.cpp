#include "utils.hpp"

namespace tria::gfx::internal {

auto getVkAvailableInstanceExtensions() -> std::vector<VkExtensionProperties> {
  uint32_t extCount = 0U;
  checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
  auto result = std::vector<VkExtensionProperties>{extCount};
  checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, result.data()));
  return result;
}

auto getVkAvailableInstanceLayers() -> std::vector<VkLayerProperties> {
  uint32_t layerCount = 0U;
  checkVkResult(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
  auto result = std::vector<VkLayerProperties>{layerCount};
  checkVkResult(vkEnumerateInstanceLayerProperties(&layerCount, result.data()));
  return result;
}

auto getVkPhysicalDevices(VkInstance vkInstance) -> std::vector<VkPhysicalDevice> {
  uint32_t count = 0;
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &count, nullptr));
  auto result = std::vector<VkPhysicalDevice>{count};
  checkVkResult(vkEnumeratePhysicalDevices(vkInstance, &count, result.data()));
  return result;
}

auto getVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice) -> std::vector<VkQueueFamilyProperties> {
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
  auto result = std::vector<VkQueueFamilyProperties>{count};
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, result.data());
  return result;
}

auto getVkDeviceExtensions(VkPhysicalDevice vkPhysicalDevice)
    -> std::vector<VkExtensionProperties> {
  uint32_t count = 0;
  checkVkResult(vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, nullptr));
  auto result = std::vector<VkExtensionProperties>{count};
  checkVkResult(
      vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, result.data()));
  return result;
}

auto getVkSurfaceFormats(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkSurfaceFormatKHR> {
  uint32_t count = 0;
  checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, nullptr));
  auto result = std::vector<VkSurfaceFormatKHR>{count};
  checkVkResult(
      vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, result.data()));
  return result;
}

auto getVkPresentModes(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    -> std::vector<VkPresentModeKHR> {
  uint32_t count = 0;
  checkVkResult(
      vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &count, nullptr));
  auto result = std::vector<VkPresentModeKHR>{count};
  checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vkPhysicalDevice, vkSurface, &count, result.data()));
  return result;
}

auto getSwapchainVkImages(VkDevice vkDevice, VkSwapchainKHR vkSwapchain) -> std::vector<VkImage> {
  uint32_t count = 0;
  checkVkResult(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &count, nullptr));
  auto result = std::vector<VkImage>{count};
  checkVkResult(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &count, result.data()));
  return result;
}

auto createVkSemaphore(VkDevice vkDevice) -> VkSemaphore {
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkSemaphore result;
  checkVkResult(vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &result));
  return result;
}

auto createVkFence(VkDevice vkDevice, bool initialState) -> VkFence {
  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags             = initialState ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
  VkFence result;
  checkVkResult(vkCreateFence(vkDevice, &fenceInfo, nullptr, &result));
  return result;
}

auto getVkErrStr(VkResult errCode) noexcept -> std::string_view {
#define ERROR_STR(NAME)                                                                            \
  case VK_##NAME:                                                                                  \
    return "Vulkan error: " #NAME

  switch (errCode) {
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "No compatible vulkan driver found";
    ERROR_STR(NOT_READY);
    ERROR_STR(TIMEOUT);
    ERROR_STR(EVENT_SET);
    ERROR_STR(EVENT_RESET);
    ERROR_STR(INCOMPLETE);
    ERROR_STR(ERROR_OUT_OF_HOST_MEMORY);
    ERROR_STR(ERROR_OUT_OF_DEVICE_MEMORY);
    ERROR_STR(ERROR_INITIALIZATION_FAILED);
    ERROR_STR(ERROR_DEVICE_LOST);
    ERROR_STR(ERROR_MEMORY_MAP_FAILED);
    ERROR_STR(ERROR_LAYER_NOT_PRESENT);
    ERROR_STR(ERROR_EXTENSION_NOT_PRESENT);
    ERROR_STR(ERROR_FEATURE_NOT_PRESENT);
    ERROR_STR(ERROR_TOO_MANY_OBJECTS);
    ERROR_STR(ERROR_FORMAT_NOT_SUPPORTED);
    ERROR_STR(ERROR_FRAGMENTED_POOL);
    ERROR_STR(ERROR_UNKNOWN);
    ERROR_STR(ERROR_OUT_OF_POOL_MEMORY);
    ERROR_STR(ERROR_INVALID_EXTERNAL_HANDLE);
    ERROR_STR(ERROR_FRAGMENTATION);
    ERROR_STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
    ERROR_STR(ERROR_SURFACE_LOST_KHR);
    ERROR_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    ERROR_STR(SUBOPTIMAL_KHR);
    ERROR_STR(ERROR_OUT_OF_DATE_KHR);
    ERROR_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    ERROR_STR(ERROR_VALIDATION_FAILED_EXT);
    ERROR_STR(ERROR_INVALID_SHADER_NV);
    ERROR_STR(ERROR_INCOMPATIBLE_VERSION_KHR);
    ERROR_STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
    ERROR_STR(ERROR_NOT_PERMITTED_EXT);
    ERROR_STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
    ERROR_STR(THREAD_IDLE_KHR);
    ERROR_STR(THREAD_DONE_KHR);
    ERROR_STR(OPERATION_DEFERRED_KHR);
    ERROR_STR(OPERATION_NOT_DEFERRED_KHR);
    ERROR_STR(PIPELINE_COMPILE_REQUIRED_EXT);
  default:
    return "Unknown vulkan error";

#undef ERROR_STR
  }
}

auto getVkDeviceTypeString(VkPhysicalDeviceType type) noexcept -> std::string_view {
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

auto getVkVendorString(uint32_t vendorId) noexcept -> std::string_view {
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

auto getVkColorSpaceString(VkColorSpaceKHR colorSpace) noexcept -> std::string_view {
#define COLORSPACE_STR(NAME)                                                                       \
  case VK_COLOR_SPACE_##NAME:                                                                      \
    return #NAME

  switch (colorSpace) {
    COLORSPACE_STR(SRGB_NONLINEAR_KHR);
    COLORSPACE_STR(DISPLAY_P3_NONLINEAR_EXT);
    COLORSPACE_STR(EXTENDED_SRGB_LINEAR_EXT);
    COLORSPACE_STR(DISPLAY_P3_LINEAR_EXT);
    COLORSPACE_STR(DCI_P3_NONLINEAR_EXT);
    COLORSPACE_STR(BT709_LINEAR_EXT);
    COLORSPACE_STR(BT709_NONLINEAR_EXT);
    COLORSPACE_STR(BT2020_LINEAR_EXT);
    COLORSPACE_STR(HDR10_ST2084_EXT);
    COLORSPACE_STR(DOLBYVISION_EXT);
    COLORSPACE_STR(HDR10_HLG_EXT);
    COLORSPACE_STR(ADOBERGB_LINEAR_EXT);
    COLORSPACE_STR(ADOBERGB_NONLINEAR_EXT);
    COLORSPACE_STR(PASS_THROUGH_EXT);
    COLORSPACE_STR(EXTENDED_SRGB_NONLINEAR_EXT);
    COLORSPACE_STR(DISPLAY_NATIVE_AMD);
  default:
    return "unknown";
  }
#undef COLORSPACE_STR
}

auto getVkPresentModeString(VkPresentModeKHR mode) noexcept -> std::string_view {
#define PRESENTMODE_STR(NAME)                                                                      \
  case VK_PRESENT_MODE_##NAME:                                                                     \
    return #NAME

  switch (mode) {
    PRESENTMODE_STR(IMMEDIATE_KHR);
    PRESENTMODE_STR(MAILBOX_KHR);
    PRESENTMODE_STR(FIFO_KHR);
    PRESENTMODE_STR(FIFO_RELAXED_KHR);
    PRESENTMODE_STR(SHARED_DEMAND_REFRESH_KHR);
    PRESENTMODE_STR(SHARED_CONTINUOUS_REFRESH_KHR);
  default:
    return "unknown";
  }
#undef PRESENTMODE_STR
}

auto getVkSampleCountString(VkSampleCountFlagBits count) noexcept -> std::string_view {
  switch (count) {
  case VK_SAMPLE_COUNT_1_BIT:
    return "X1";
  case VK_SAMPLE_COUNT_2_BIT:
    return "X2";
  case VK_SAMPLE_COUNT_4_BIT:
    return "X4";
  case VK_SAMPLE_COUNT_8_BIT:
    return "X8";
  case VK_SAMPLE_COUNT_16_BIT:
    return "X16";
  case VK_SAMPLE_COUNT_32_BIT:
    return "X32";
  case VK_SAMPLE_COUNT_64_BIT:
    return "X64";
  default:
    return "unknown";
  }
}

auto getVkFormatInfo(VkFormat format) noexcept -> VkFormatInfo {
#define FORMAT_INFO(NAME, SIZE, CHANNEL_COUNT)                                                     \
  case VK_FORMAT_##NAME:                                                                           \
    return VkFormatInfo { #NAME, SIZE, CHANNEL_COUNT }

  switch (format) {
    FORMAT_INFO(R4G4_UNORM_PACK8, 1, 2);
    FORMAT_INFO(R4G4B4A4_UNORM_PACK16, 2, 4);
    FORMAT_INFO(B4G4R4A4_UNORM_PACK16, 2, 4);
    FORMAT_INFO(R5G6B5_UNORM_PACK16, 2, 3);
    FORMAT_INFO(B5G6R5_UNORM_PACK16, 2, 3);
    FORMAT_INFO(R5G5B5A1_UNORM_PACK16, 2, 4);
    FORMAT_INFO(B5G5R5A1_UNORM_PACK16, 2, 4);
    FORMAT_INFO(A1R5G5B5_UNORM_PACK16, 2, 4);
    FORMAT_INFO(R8_UNORM, 1, 1);
    FORMAT_INFO(R8_SNORM, 1, 1);
    FORMAT_INFO(R8_USCALED, 1, 1);
    FORMAT_INFO(R8_SSCALED, 1, 1);
    FORMAT_INFO(R8_UINT, 1, 1);
    FORMAT_INFO(R8_SINT, 1, 1);
    FORMAT_INFO(R8_SRGB, 1, 1);
    FORMAT_INFO(R8G8_UNORM, 2, 2);
    FORMAT_INFO(R8G8_SNORM, 2, 2);
    FORMAT_INFO(R8G8_USCALED, 2, 2);
    FORMAT_INFO(R8G8_SSCALED, 2, 2);
    FORMAT_INFO(R8G8_UINT, 2, 2);
    FORMAT_INFO(R8G8_SINT, 2, 2);
    FORMAT_INFO(R8G8_SRGB, 2, 2);
    FORMAT_INFO(R8G8B8_UNORM, 3, 3);
    FORMAT_INFO(R8G8B8_SNORM, 3, 3);
    FORMAT_INFO(R8G8B8_USCALED, 3, 3);
    FORMAT_INFO(R8G8B8_SSCALED, 3, 3);
    FORMAT_INFO(R8G8B8_UINT, 3, 3);
    FORMAT_INFO(R8G8B8_SINT, 3, 3);
    FORMAT_INFO(R8G8B8_SRGB, 3, 3);
    FORMAT_INFO(B8G8R8_UNORM, 3, 3);
    FORMAT_INFO(B8G8R8_SNORM, 3, 3);
    FORMAT_INFO(B8G8R8_USCALED, 3, 3);
    FORMAT_INFO(B8G8R8_SSCALED, 3, 3);
    FORMAT_INFO(B8G8R8_UINT, 3, 3);
    FORMAT_INFO(B8G8R8_SINT, 3, 3);
    FORMAT_INFO(B8G8R8_SRGB, 3, 3);
    FORMAT_INFO(R8G8B8A8_UNORM, 4, 4);
    FORMAT_INFO(R8G8B8A8_SNORM, 4, 4);
    FORMAT_INFO(R8G8B8A8_USCALED, 4, 4);
    FORMAT_INFO(R8G8B8A8_SSCALED, 4, 4);
    FORMAT_INFO(R8G8B8A8_UINT, 4, 4);
    FORMAT_INFO(R8G8B8A8_SINT, 4, 4);
    FORMAT_INFO(R8G8B8A8_SRGB, 4, 4);
    FORMAT_INFO(B8G8R8A8_UNORM, 4, 4);
    FORMAT_INFO(B8G8R8A8_SNORM, 4, 4);
    FORMAT_INFO(B8G8R8A8_USCALED, 4, 4);
    FORMAT_INFO(B8G8R8A8_SSCALED, 4, 4);
    FORMAT_INFO(B8G8R8A8_UINT, 4, 4);
    FORMAT_INFO(B8G8R8A8_SINT, 4, 4);
    FORMAT_INFO(B8G8R8A8_SRGB, 4, 4);
    FORMAT_INFO(A8B8G8R8_UNORM_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_SNORM_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_USCALED_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_SSCALED_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_UINT_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_SINT_PACK32, 4, 4);
    FORMAT_INFO(A8B8G8R8_SRGB_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_UNORM_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_SNORM_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_USCALED_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_SSCALED_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_UINT_PACK32, 4, 4);
    FORMAT_INFO(A2R10G10B10_SINT_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_UNORM_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_SNORM_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_USCALED_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_SSCALED_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_UINT_PACK32, 4, 4);
    FORMAT_INFO(A2B10G10R10_SINT_PACK32, 4, 4);
    FORMAT_INFO(R16_UNORM, 2, 1);
    FORMAT_INFO(R16_SNORM, 2, 1);
    FORMAT_INFO(R16_USCALED, 2, 1);
    FORMAT_INFO(R16_SSCALED, 2, 1);
    FORMAT_INFO(R16_UINT, 2, 1);
    FORMAT_INFO(R16_SINT, 2, 1);
    FORMAT_INFO(R16_SFLOAT, 2, 1);
    FORMAT_INFO(R16G16_UNORM, 4, 2);
    FORMAT_INFO(R16G16_SNORM, 4, 2);
    FORMAT_INFO(R16G16_USCALED, 4, 2);
    FORMAT_INFO(R16G16_SSCALED, 4, 2);
    FORMAT_INFO(R16G16_UINT, 4, 2);
    FORMAT_INFO(R16G16_SINT, 4, 2);
    FORMAT_INFO(R16G16_SFLOAT, 4, 2);
    FORMAT_INFO(R16G16B16_UNORM, 6, 3);
    FORMAT_INFO(R16G16B16_SNORM, 6, 3);
    FORMAT_INFO(R16G16B16_USCALED, 6, 3);
    FORMAT_INFO(R16G16B16_SSCALED, 6, 3);
    FORMAT_INFO(R16G16B16_UINT, 6, 3);
    FORMAT_INFO(R16G16B16_SINT, 6, 3);
    FORMAT_INFO(R16G16B16_SFLOAT, 6, 3);
    FORMAT_INFO(R16G16B16A16_UNORM, 8, 4);
    FORMAT_INFO(R16G16B16A16_SNORM, 8, 4);
    FORMAT_INFO(R16G16B16A16_USCALED, 8, 4);
    FORMAT_INFO(R16G16B16A16_SSCALED, 8, 4);
    FORMAT_INFO(R16G16B16A16_UINT, 8, 4);
    FORMAT_INFO(R16G16B16A16_SINT, 8, 4);
    FORMAT_INFO(R16G16B16A16_SFLOAT, 8, 4);
    FORMAT_INFO(R32_UINT, 4, 1);
    FORMAT_INFO(R32_SINT, 4, 1);
    FORMAT_INFO(R32_SFLOAT, 4, 1);
    FORMAT_INFO(R32G32_UINT, 8, 2);
    FORMAT_INFO(R32G32_SINT, 8, 2);
    FORMAT_INFO(R32G32_SFLOAT, 8, 2);
    FORMAT_INFO(R32G32B32_UINT, 2, 3);
    FORMAT_INFO(R32G32B32_SINT, 2, 3);
    FORMAT_INFO(R32G32B32_SFLOAT, 2, 3);
    FORMAT_INFO(R32G32B32A32_UINT, 6, 4);
    FORMAT_INFO(R32G32B32A32_SINT, 6, 4);
    FORMAT_INFO(R32G32B32A32_SFLOAT, 6, 4);
    FORMAT_INFO(R64_UINT, 8, 1);
    FORMAT_INFO(R64_SINT, 8, 1);
    FORMAT_INFO(R64_SFLOAT, 8, 1);
    FORMAT_INFO(R64G64_UINT, 6, 2);
    FORMAT_INFO(R64G64_SINT, 6, 2);
    FORMAT_INFO(R64G64_SFLOAT, 6, 2);
    FORMAT_INFO(R64G64B64_UINT, 4, 3);
    FORMAT_INFO(R64G64B64_SINT, 4, 3);
    FORMAT_INFO(R64G64B64_SFLOAT, 4, 3);
    FORMAT_INFO(R64G64B64A64_UINT, 2, 4);
    FORMAT_INFO(R64G64B64A64_SINT, 2, 4);
    FORMAT_INFO(R64G64B64A64_SFLOAT, 2, 4);
    FORMAT_INFO(B10G11R11_UFLOAT_PACK32, 4, 3);
    FORMAT_INFO(E5B9G9R9_UFLOAT_PACK32, 4, 3);
    FORMAT_INFO(D16_UNORM, 2, 1);
    FORMAT_INFO(X8_D24_UNORM_PACK32, 4, 1);
    FORMAT_INFO(D32_SFLOAT, 4, 1);
    FORMAT_INFO(S8_UINT, 1, 1);
    FORMAT_INFO(D16_UNORM_S8_UINT, 3, 2);
    FORMAT_INFO(D24_UNORM_S8_UINT, 4, 2);
    FORMAT_INFO(D32_SFLOAT_S8_UINT, 8, 2);
    FORMAT_INFO(BC1_RGB_UNORM_BLOCK, 8, 4);
    FORMAT_INFO(BC1_RGB_SRGB_BLOCK, 8, 4);
    FORMAT_INFO(BC1_RGBA_UNORM_BLOCK, 8, 4);
    FORMAT_INFO(BC1_RGBA_SRGB_BLOCK, 8, 4);
    FORMAT_INFO(BC2_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(BC2_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(BC3_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(BC3_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(BC4_UNORM_BLOCK, 8, 4);
    FORMAT_INFO(BC4_SNORM_BLOCK, 8, 4);
    FORMAT_INFO(BC5_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(BC5_SNORM_BLOCK, 6, 4);
    FORMAT_INFO(BC6H_UFLOAT_BLOCK, 6, 4);
    FORMAT_INFO(BC6H_SFLOAT_BLOCK, 6, 4);
    FORMAT_INFO(BC7_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(BC7_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ETC2_R8G8B8_UNORM_BLOCK, 8, 3);
    FORMAT_INFO(ETC2_R8G8B8_SRGB_BLOCK, 8, 3);
    FORMAT_INFO(ETC2_R8G8B8A1_UNORM_BLOCK, 8, 4);
    FORMAT_INFO(ETC2_R8G8B8A1_SRGB_BLOCK, 8, 4);
    FORMAT_INFO(ETC2_R8G8B8A8_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ETC2_R8G8B8A8_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(EAC_R11_UNORM_BLOCK, 8, 1);
    FORMAT_INFO(EAC_R11_SNORM_BLOCK, 8, 1);
    FORMAT_INFO(EAC_R11G11_UNORM_BLOCK, 6, 2);
    FORMAT_INFO(EAC_R11G11_SNORM_BLOCK, 6, 2);
    FORMAT_INFO(ASTC_4x4_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_4x4_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_5x4_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_5x4_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_5x5_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_5x5_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_6x5_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_6x5_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_6x6_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_6x6_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x5_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x5_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x6_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x6_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x8_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_8x8_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x5_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x5_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x6_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x6_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x8_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x8_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x10_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_10x10_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_12x10_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_12x10_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_12x12_UNORM_BLOCK, 6, 4);
    FORMAT_INFO(ASTC_12x12_SRGB_BLOCK, 6, 4);
    FORMAT_INFO(PVRTC1_2BPP_UNORM_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC1_4BPP_UNORM_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC2_2BPP_UNORM_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC2_4BPP_UNORM_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC1_2BPP_SRGB_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC1_4BPP_SRGB_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC2_2BPP_SRGB_BLOCK_IMG, 8, 4);
    FORMAT_INFO(PVRTC2_4BPP_SRGB_BLOCK_IMG, 8, 4);
    FORMAT_INFO(R10X6_UNORM_PACK16, 2, 1);
    FORMAT_INFO(R10X6G10X6_UNORM_2PACK16, 4, 2);
    FORMAT_INFO(R10X6G10X6B10X6A10X6_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(R12X4_UNORM_PACK16, 2, 1);
    FORMAT_INFO(R12X4G12X4_UNORM_2PACK16, 4, 2);
    FORMAT_INFO(R12X4G12X4B12X4A12X4_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(G8B8G8R8_422_UNORM, 4, 4);
    FORMAT_INFO(B8G8R8G8_422_UNORM, 4, 4);
    FORMAT_INFO(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, 8, 4);
    FORMAT_INFO(G16B16G16R16_422_UNORM, 8, 4);
    FORMAT_INFO(B16G16R16G16_422_UNORM, 8, 4);
    FORMAT_INFO(G8_B8_R8_3PLANE_420_UNORM, 6, 3);
    FORMAT_INFO(G8_B8R8_2PLANE_420_UNORM, 6, 3);
    FORMAT_INFO(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, 2, 3);
    FORMAT_INFO(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, 2, 3);
    FORMAT_INFO(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, 2, 3);
    FORMAT_INFO(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, 2, 3);
    FORMAT_INFO(G16_B16_R16_3PLANE_420_UNORM, 2, 3);
    FORMAT_INFO(G16_B16R16_2PLANE_420_UNORM, 2, 3);
    FORMAT_INFO(G8_B8_R8_3PLANE_422_UNORM, 4, 3);
    FORMAT_INFO(G8_B8R8_2PLANE_422_UNORM, 4, 3);
    FORMAT_INFO(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, 8, 3);
    FORMAT_INFO(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, 8, 3);
    FORMAT_INFO(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, 8, 3);
    FORMAT_INFO(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, 8, 3);
    FORMAT_INFO(G16_B16_R16_3PLANE_422_UNORM, 8, 3);
    FORMAT_INFO(G16_B16R16_2PLANE_422_UNORM, 8, 3);
    FORMAT_INFO(G8_B8_R8_3PLANE_444_UNORM, 3, 3);
    FORMAT_INFO(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, 6, 3);
    FORMAT_INFO(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, 6, 3);
    FORMAT_INFO(G16_B16_R16_3PLANE_444_UNORM, 6, 3);
  default:
    return VkFormatInfo{"uknown", 0U, 0U};
  }
#undef FORMAT_INFO
}

} // namespace tria::gfx::internal

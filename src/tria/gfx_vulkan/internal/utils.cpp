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

auto getVkErrStr(VkResult errCode) noexcept -> std::string {
#define ERROR_STR(name)                                                                            \
  case VK_##name:                                                                                  \
    return "Vulkan error: " #name

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

auto getVkDeviceTypeString(VkPhysicalDeviceType type) noexcept -> std::string {
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

auto getVkVendorString(uint32_t vendorId) noexcept -> std::string {
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

auto getVkFormatString(VkFormat format) noexcept -> std::string {
#define FORMAT_STR(name)                                                                           \
  case VK_FORMAT_##name:                                                                           \
    return #name

  switch (format) {
    FORMAT_STR(UNDEFINED);
    FORMAT_STR(R4G4_UNORM_PACK8);
    FORMAT_STR(R4G4B4A4_UNORM_PACK16);
    FORMAT_STR(B4G4R4A4_UNORM_PACK16);
    FORMAT_STR(R5G6B5_UNORM_PACK16);
    FORMAT_STR(B5G6R5_UNORM_PACK16);
    FORMAT_STR(R5G5B5A1_UNORM_PACK16);
    FORMAT_STR(B5G5R5A1_UNORM_PACK16);
    FORMAT_STR(A1R5G5B5_UNORM_PACK16);
    FORMAT_STR(R8_UNORM);
    FORMAT_STR(R8_SNORM);
    FORMAT_STR(R8_USCALED);
    FORMAT_STR(R8_SSCALED);
    FORMAT_STR(R8_UINT);
    FORMAT_STR(R8_SINT);
    FORMAT_STR(R8_SRGB);
    FORMAT_STR(R8G8_UNORM);
    FORMAT_STR(R8G8_SNORM);
    FORMAT_STR(R8G8_USCALED);
    FORMAT_STR(R8G8_SSCALED);
    FORMAT_STR(R8G8_UINT);
    FORMAT_STR(R8G8_SINT);
    FORMAT_STR(R8G8_SRGB);
    FORMAT_STR(R8G8B8_UNORM);
    FORMAT_STR(R8G8B8_SNORM);
    FORMAT_STR(R8G8B8_USCALED);
    FORMAT_STR(R8G8B8_SSCALED);
    FORMAT_STR(R8G8B8_UINT);
    FORMAT_STR(R8G8B8_SINT);
    FORMAT_STR(R8G8B8_SRGB);
    FORMAT_STR(B8G8R8_UNORM);
    FORMAT_STR(B8G8R8_SNORM);
    FORMAT_STR(B8G8R8_USCALED);
    FORMAT_STR(B8G8R8_SSCALED);
    FORMAT_STR(B8G8R8_UINT);
    FORMAT_STR(B8G8R8_SINT);
    FORMAT_STR(B8G8R8_SRGB);
    FORMAT_STR(R8G8B8A8_UNORM);
    FORMAT_STR(R8G8B8A8_SNORM);
    FORMAT_STR(R8G8B8A8_USCALED);
    FORMAT_STR(R8G8B8A8_SSCALED);
    FORMAT_STR(R8G8B8A8_UINT);
    FORMAT_STR(R8G8B8A8_SINT);
    FORMAT_STR(R8G8B8A8_SRGB);
    FORMAT_STR(B8G8R8A8_UNORM);
    FORMAT_STR(B8G8R8A8_SNORM);
    FORMAT_STR(B8G8R8A8_USCALED);
    FORMAT_STR(B8G8R8A8_SSCALED);
    FORMAT_STR(B8G8R8A8_UINT);
    FORMAT_STR(B8G8R8A8_SINT);
    FORMAT_STR(B8G8R8A8_SRGB);
    FORMAT_STR(A8B8G8R8_UNORM_PACK32);
    FORMAT_STR(A8B8G8R8_SNORM_PACK32);
    FORMAT_STR(A8B8G8R8_USCALED_PACK32);
    FORMAT_STR(A8B8G8R8_SSCALED_PACK32);
    FORMAT_STR(A8B8G8R8_UINT_PACK32);
    FORMAT_STR(A8B8G8R8_SINT_PACK32);
    FORMAT_STR(A8B8G8R8_SRGB_PACK32);
    FORMAT_STR(A2R10G10B10_UNORM_PACK32);
    FORMAT_STR(A2R10G10B10_SNORM_PACK32);
    FORMAT_STR(A2R10G10B10_USCALED_PACK32);
    FORMAT_STR(A2R10G10B10_SSCALED_PACK32);
    FORMAT_STR(A2R10G10B10_UINT_PACK32);
    FORMAT_STR(A2R10G10B10_SINT_PACK32);
    FORMAT_STR(A2B10G10R10_UNORM_PACK32);
    FORMAT_STR(A2B10G10R10_SNORM_PACK32);
    FORMAT_STR(A2B10G10R10_USCALED_PACK32);
    FORMAT_STR(A2B10G10R10_SSCALED_PACK32);
    FORMAT_STR(A2B10G10R10_UINT_PACK32);
    FORMAT_STR(A2B10G10R10_SINT_PACK32);
    FORMAT_STR(R16_UNORM);
    FORMAT_STR(R16_SNORM);
    FORMAT_STR(R16_USCALED);
    FORMAT_STR(R16_SSCALED);
    FORMAT_STR(R16_UINT);
    FORMAT_STR(R16_SINT);
    FORMAT_STR(R16_SFLOAT);
    FORMAT_STR(R16G16_UNORM);
    FORMAT_STR(R16G16_SNORM);
    FORMAT_STR(R16G16_USCALED);
    FORMAT_STR(R16G16_SSCALED);
    FORMAT_STR(R16G16_UINT);
    FORMAT_STR(R16G16_SINT);
    FORMAT_STR(R16G16_SFLOAT);
    FORMAT_STR(R16G16B16_UNORM);
    FORMAT_STR(R16G16B16_SNORM);
    FORMAT_STR(R16G16B16_USCALED);
    FORMAT_STR(R16G16B16_SSCALED);
    FORMAT_STR(R16G16B16_UINT);
    FORMAT_STR(R16G16B16_SINT);
    FORMAT_STR(R16G16B16_SFLOAT);
    FORMAT_STR(R16G16B16A16_UNORM);
    FORMAT_STR(R16G16B16A16_SNORM);
    FORMAT_STR(R16G16B16A16_USCALED);
    FORMAT_STR(R16G16B16A16_SSCALED);
    FORMAT_STR(R16G16B16A16_UINT);
    FORMAT_STR(R16G16B16A16_SINT);
    FORMAT_STR(R16G16B16A16_SFLOAT);
    FORMAT_STR(R32_UINT);
    FORMAT_STR(R32_SINT);
    FORMAT_STR(R32_SFLOAT);
    FORMAT_STR(R32G32_UINT);
    FORMAT_STR(R32G32_SINT);
    FORMAT_STR(R32G32_SFLOAT);
    FORMAT_STR(R32G32B32_UINT);
    FORMAT_STR(R32G32B32_SINT);
    FORMAT_STR(R32G32B32_SFLOAT);
    FORMAT_STR(R32G32B32A32_UINT);
    FORMAT_STR(R32G32B32A32_SINT);
    FORMAT_STR(R32G32B32A32_SFLOAT);
    FORMAT_STR(R64_UINT);
    FORMAT_STR(R64_SINT);
    FORMAT_STR(R64_SFLOAT);
    FORMAT_STR(R64G64_UINT);
    FORMAT_STR(R64G64_SINT);
    FORMAT_STR(R64G64_SFLOAT);
    FORMAT_STR(R64G64B64_UINT);
    FORMAT_STR(R64G64B64_SINT);
    FORMAT_STR(R64G64B64_SFLOAT);
    FORMAT_STR(R64G64B64A64_UINT);
    FORMAT_STR(R64G64B64A64_SINT);
    FORMAT_STR(R64G64B64A64_SFLOAT);
    FORMAT_STR(B10G11R11_UFLOAT_PACK32);
    FORMAT_STR(E5B9G9R9_UFLOAT_PACK32);
    FORMAT_STR(D16_UNORM);
    FORMAT_STR(X8_D24_UNORM_PACK32);
    FORMAT_STR(D32_SFLOAT);
    FORMAT_STR(S8_UINT);
    FORMAT_STR(D16_UNORM_S8_UINT);
    FORMAT_STR(D24_UNORM_S8_UINT);
    FORMAT_STR(D32_SFLOAT_S8_UINT);
    FORMAT_STR(BC1_RGB_UNORM_BLOCK);
    FORMAT_STR(BC1_RGB_SRGB_BLOCK);
    FORMAT_STR(BC1_RGBA_UNORM_BLOCK);
    FORMAT_STR(BC1_RGBA_SRGB_BLOCK);
    FORMAT_STR(BC2_UNORM_BLOCK);
    FORMAT_STR(BC2_SRGB_BLOCK);
    FORMAT_STR(BC3_UNORM_BLOCK);
    FORMAT_STR(BC3_SRGB_BLOCK);
    FORMAT_STR(BC4_UNORM_BLOCK);
    FORMAT_STR(BC4_SNORM_BLOCK);
    FORMAT_STR(BC5_UNORM_BLOCK);
    FORMAT_STR(BC5_SNORM_BLOCK);
    FORMAT_STR(BC6H_UFLOAT_BLOCK);
    FORMAT_STR(BC6H_SFLOAT_BLOCK);
    FORMAT_STR(BC7_UNORM_BLOCK);
    FORMAT_STR(BC7_SRGB_BLOCK);
    FORMAT_STR(ETC2_R8G8B8_UNORM_BLOCK);
    FORMAT_STR(ETC2_R8G8B8_SRGB_BLOCK);
    FORMAT_STR(ETC2_R8G8B8A1_UNORM_BLOCK);
    FORMAT_STR(ETC2_R8G8B8A1_SRGB_BLOCK);
    FORMAT_STR(ETC2_R8G8B8A8_UNORM_BLOCK);
    FORMAT_STR(ETC2_R8G8B8A8_SRGB_BLOCK);
    FORMAT_STR(EAC_R11_UNORM_BLOCK);
    FORMAT_STR(EAC_R11_SNORM_BLOCK);
    FORMAT_STR(EAC_R11G11_UNORM_BLOCK);
    FORMAT_STR(EAC_R11G11_SNORM_BLOCK);
    FORMAT_STR(ASTC_4x4_UNORM_BLOCK);
    FORMAT_STR(ASTC_4x4_SRGB_BLOCK);
    FORMAT_STR(ASTC_5x4_UNORM_BLOCK);
    FORMAT_STR(ASTC_5x4_SRGB_BLOCK);
    FORMAT_STR(ASTC_5x5_UNORM_BLOCK);
    FORMAT_STR(ASTC_5x5_SRGB_BLOCK);
    FORMAT_STR(ASTC_6x5_UNORM_BLOCK);
    FORMAT_STR(ASTC_6x5_SRGB_BLOCK);
    FORMAT_STR(ASTC_6x6_UNORM_BLOCK);
    FORMAT_STR(ASTC_6x6_SRGB_BLOCK);
    FORMAT_STR(ASTC_8x5_UNORM_BLOCK);
    FORMAT_STR(ASTC_8x5_SRGB_BLOCK);
    FORMAT_STR(ASTC_8x6_UNORM_BLOCK);
    FORMAT_STR(ASTC_8x6_SRGB_BLOCK);
    FORMAT_STR(ASTC_8x8_UNORM_BLOCK);
    FORMAT_STR(ASTC_8x8_SRGB_BLOCK);
    FORMAT_STR(ASTC_10x5_UNORM_BLOCK);
    FORMAT_STR(ASTC_10x5_SRGB_BLOCK);
    FORMAT_STR(ASTC_10x6_UNORM_BLOCK);
    FORMAT_STR(ASTC_10x6_SRGB_BLOCK);
    FORMAT_STR(ASTC_10x8_UNORM_BLOCK);
    FORMAT_STR(ASTC_10x8_SRGB_BLOCK);
    FORMAT_STR(ASTC_10x10_UNORM_BLOCK);
    FORMAT_STR(ASTC_10x10_SRGB_BLOCK);
    FORMAT_STR(ASTC_12x10_UNORM_BLOCK);
    FORMAT_STR(ASTC_12x10_SRGB_BLOCK);
    FORMAT_STR(ASTC_12x12_UNORM_BLOCK);
    FORMAT_STR(ASTC_12x12_SRGB_BLOCK);
    FORMAT_STR(G8B8G8R8_422_UNORM);
    FORMAT_STR(B8G8R8G8_422_UNORM);
    FORMAT_STR(G8_B8_R8_3PLANE_420_UNORM);
    FORMAT_STR(G8_B8R8_2PLANE_420_UNORM);
    FORMAT_STR(G8_B8_R8_3PLANE_422_UNORM);
    FORMAT_STR(G8_B8R8_2PLANE_422_UNORM);
    FORMAT_STR(G8_B8_R8_3PLANE_444_UNORM);
    FORMAT_STR(R10X6_UNORM_PACK16);
    FORMAT_STR(R10X6G10X6_UNORM_2PACK16);
    FORMAT_STR(R10X6G10X6B10X6A10X6_UNORM_4PACK16);
    FORMAT_STR(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
    FORMAT_STR(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
    FORMAT_STR(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
    FORMAT_STR(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
    FORMAT_STR(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
    FORMAT_STR(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
    FORMAT_STR(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
    FORMAT_STR(R12X4_UNORM_PACK16);
    FORMAT_STR(R12X4G12X4_UNORM_2PACK16);
    FORMAT_STR(R12X4G12X4B12X4A12X4_UNORM_4PACK16);
    FORMAT_STR(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
    FORMAT_STR(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
    FORMAT_STR(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
    FORMAT_STR(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
    FORMAT_STR(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
    FORMAT_STR(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
    FORMAT_STR(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
    FORMAT_STR(G16B16G16R16_422_UNORM);
    FORMAT_STR(B16G16R16G16_422_UNORM);
    FORMAT_STR(G16_B16_R16_3PLANE_420_UNORM);
    FORMAT_STR(G16_B16R16_2PLANE_420_UNORM);
    FORMAT_STR(G16_B16_R16_3PLANE_422_UNORM);
    FORMAT_STR(G16_B16R16_2PLANE_422_UNORM);
    FORMAT_STR(G16_B16_R16_3PLANE_444_UNORM);
    FORMAT_STR(PVRTC1_2BPP_UNORM_BLOCK_IMG);
    FORMAT_STR(PVRTC1_4BPP_UNORM_BLOCK_IMG);
    FORMAT_STR(PVRTC2_2BPP_UNORM_BLOCK_IMG);
    FORMAT_STR(PVRTC2_4BPP_UNORM_BLOCK_IMG);
    FORMAT_STR(PVRTC1_2BPP_SRGB_BLOCK_IMG);
    FORMAT_STR(PVRTC1_4BPP_SRGB_BLOCK_IMG);
    FORMAT_STR(PVRTC2_2BPP_SRGB_BLOCK_IMG);
    FORMAT_STR(PVRTC2_4BPP_SRGB_BLOCK_IMG);
    FORMAT_STR(ASTC_4x4_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_5x4_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_5x5_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_6x5_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_6x6_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_8x5_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_8x6_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_8x8_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_10x5_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_10x6_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_10x8_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_10x10_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_12x10_SFLOAT_BLOCK_EXT);
    FORMAT_STR(ASTC_12x12_SFLOAT_BLOCK_EXT);
  default:
    return "unknown";
  }
#undef FORMAT_STR
}

auto getVkColorSpaceString(VkColorSpaceKHR colorSpace) noexcept -> std::string {
#define COLORSPACE_STR(name)                                                                       \
  case VK_COLOR_SPACE_##name:                                                                      \
    return #name

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

auto getVkPresentModeString(VkPresentModeKHR mode) noexcept -> std::string {
#define PRESENTMODE_STR(name)                                                                      \
  case VK_PRESENT_MODE_##name:                                                                     \
    return #name

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

} // namespace tria::gfx::internal

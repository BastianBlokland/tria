#include "debug_messenger.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

// Called by the vulkan driver.
static auto vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) noexcept -> VkBool32 {

  auto* messenger = static_cast<DebugMessenger*>(pUserData);
  messenger->handleMessage(messageSeverity, messageType, pCallbackData);
  return false;
}

namespace {

// Proxy function that dynamically loads the underlying vulkan function.
auto createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult {

  const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  return func ? func(instance, pCreateInfo, pAllocator, pDebugMessenger)
              : VK_ERROR_EXTENSION_NOT_PRESENT;
}

// Proxy function that dynamically loads the underlying vulkan function.
auto destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) -> void {

  const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (func) {
    func(instance, debugMessenger, pAllocator);
  }
}

auto setupVkDebugMessenger(VkInstance vkInstance, DebugMessenger* messenger, bool verbose)
    -> VkDebugUtilsMessengerEXT {
  assert(messenger);

  auto severityMask = 0U;
  if (verbose) {
    severityMask = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  } else {
    severityMask = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
  createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = severityMask;
  createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = vkDebugCallback;
  createInfo.pUserData       = static_cast<void*>(messenger);

  VkDebugUtilsMessengerEXT result;
  checkVkResult(createDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &result));
  return result;
}

} // namespace

DebugMessenger::DebugMessenger(log::Logger* logger, VkInstance vkInstance, bool verbose) :
    m_logger{logger}, m_vkInstance{vkInstance} {

  m_vkDebugMessenger = setupVkDebugMessenger(m_vkInstance, this, verbose);
}

DebugMessenger::~DebugMessenger() {
  destroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
}

auto DebugMessenger::handleMessage(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) noexcept -> void {

  const char* messageTypeLabel = nullptr;
  switch (messageType) {
  case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
    messageTypeLabel = "general";
    break;
  case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    messageTypeLabel = "validation";
    break;
  case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    messageTypeLabel = "performance";
    break;
  default:
    messageTypeLabel = "unknown";
    break;
  }

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    LOG_E(
        m_logger,
        "Vulkan validation error",
        {"type", messageTypeLabel},
        {"message", pCallbackData->pMessage});
  } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    LOG_W(
        m_logger,
        "Vulkan validation warning",
        {"type", messageTypeLabel},
        {"message", pCallbackData->pMessage});
  } else {
    LOG_D(
        m_logger,
        "Vulkan validation message",
        {"type", messageTypeLabel},
        {"message", pCallbackData->pMessage});
  }
}

} // namespace tria::gfx::internal

#pragma once
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class DebugMessenger final {
public:
  DebugMessenger(VkInstance vkInstance, log::Logger* logger);
  ~DebugMessenger();

  auto handleMessage(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) -> void;

private:
  VkInstance m_vkInstance;
  log::Logger* m_logger;
  VkDebugUtilsMessengerEXT m_vkDebugMessenger;
};

} // namespace tria::gfx::internal

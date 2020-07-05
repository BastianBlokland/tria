#pragma once
#include "tria/log/api.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/*
 * DebugMessenger can be used to receive and log diagnostics from the vulkan validation layers.
 */
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

using DebugMessengerPtr = std::unique_ptr<DebugMessenger>;

} // namespace tria::gfx::internal

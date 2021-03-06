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
  DebugMessenger(log::Logger* logger, VkInstance vkInstance, bool verbose);
  DebugMessenger(const DebugMessenger& rhs)     = delete;
  DebugMessenger(DebugMessenger&& rhs) noexcept = delete;
  ~DebugMessenger();

  auto operator=(const DebugMessenger& rhs) -> DebugMessenger& = delete;
  auto operator=(DebugMessenger&& rhs) noexcept -> DebugMessenger& = delete;

  auto handleMessage(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) noexcept -> void;

private:
  log::Logger* m_logger;
  VkInstance m_vkInstance;
  VkDebugUtilsMessengerEXT m_vkDebugMessenger;
};

using DebugMessengerUnique = std::unique_ptr<DebugMessenger>;

} // namespace tria::gfx::internal

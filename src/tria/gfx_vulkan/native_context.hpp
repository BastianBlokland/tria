#pragma once
#include "internal/debug_messenger.hpp"
#include "internal/device.hpp"
#include "tria/gfx/context.hpp"
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeContext final {
public:
  NativeContext(log::Logger* logger);
  ~NativeContext();

private:
  log::Logger* m_logger;
  std::string m_appName;
  VkInstance m_vkInstance;
  internal::DebugMessengerPtr m_dbgMessenger;
  internal::DevicePtr m_device;
};

} // namespace tria::gfx

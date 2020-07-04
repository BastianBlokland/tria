#pragma once
#include "internal/debug_messenger.hpp"
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
  std::unique_ptr<internal::DebugMessenger> m_dbgMessenger;
};

} // namespace tria::gfx

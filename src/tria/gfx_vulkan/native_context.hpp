#pragma once
#include "internal/debug_messenger.hpp"
#include "tria/gfx/context.hpp"
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeSurface;

class NativeContext final {
public:
  NativeContext(log::Logger* logger);
  ~NativeContext();

  [[nodiscard]] auto getVkInstance() const noexcept { return m_vkInstance; }

  [[nodiscard]] auto createSurface(const pal::Window* window) -> std::unique_ptr<NativeSurface>;

private:
  log::Logger* m_logger;
  std::string m_appName;
  VkInstance m_vkInstance;
  internal::DebugMessengerPtr m_dbgMessenger;
};

} // namespace tria::gfx

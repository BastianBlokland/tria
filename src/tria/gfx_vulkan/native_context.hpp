#pragma once
#include "internal/debug_messenger.hpp"
#include "tria/gfx/context.hpp"
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeCanvas;

/*
 * Implementation of a graphics 'Context' abstraction.
 */
class NativeContext final {
public:
  NativeContext(log::Logger* logger);
  ~NativeContext();

  [[nodiscard]] auto getVkInstance() const noexcept { return m_vkInstance; }

  [[nodiscard]] auto createCanvas(const pal::Window* window, VSyncMode vSync)
      -> std::unique_ptr<NativeCanvas>;

private:
  log::Logger* m_logger;
  std::string m_appName;
  VkInstance m_vkInstance;
  internal::DebugMessengerUnique m_dbgMessenger;
};

} // namespace tria::gfx

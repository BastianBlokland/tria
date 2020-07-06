#pragma once
#include "internal/device.hpp"
#include "tria/gfx/surface.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeContext;

class NativeSurface final {
public:
  NativeSurface(log::Logger* logger, const NativeContext* context, const pal::Window* window);
  ~NativeSurface();

private:
  log::Logger* m_logger;
  const NativeContext* m_context;
  const pal::Window* m_window;
  VkSurfaceKHR m_vkSurface;
  internal::DevicePtr m_device;
};

} // namespace tria::gfx

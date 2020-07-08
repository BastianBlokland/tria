#include "native_surface.hpp"
#include "internal/utils.hpp"
#include "native_context.hpp"
#include "tria/pal/native.hpp"
#include "tria/pal/utils.hpp"
#include <cassert>

namespace tria::gfx {

using namespace internal;

namespace {

[[nodiscard]] auto makeVkSurfaceKhr(const NativeContext* context, const pal::Window* window)
    -> VkSurfaceKHR {
  VkSurfaceKHR result;
#if defined(TRIA_LINUX_XCB)
  VkXcbSurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.connection                = pal::getLinuxXcbConnection(*window);
  createInfo.window                    = pal::getLinuxXcbWindow(*window);
  checkVkResult(vkCreateXcbSurfaceKHR(context->getVkInstance(), &createInfo, nullptr, &result));
#elif defined(TRIA_WIN32)
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance                   = pal::getWin32HInstance(*window);
  createInfo.hwnd                        = pal::getWin32HWnd(*window);
  checkVkResult(vkCreateWin32SurfaceKHR(context->getVkInstance(), &createInfo, nullptr, &result));
#else
  static_assert("false", "Unsupported platform");
#endif
  return result;
}

} // namespace

NativeSurface::NativeSurface(
    log::Logger* logger, const NativeContext* context, const pal::Window* window) :
    m_logger{logger}, m_context{context}, m_window{window} {
  assert(m_context);
  assert(m_window);

  m_vkSurface = makeVkSurfaceKhr(m_context, m_window);
  try {
    m_device = getDevice(m_logger, context->getVkInstance(), m_vkSurface);
    if (!m_device) {
      throw err::DriverErr{"No device found with vulkan support"};
    }
  } catch (...) {
    // Cleanup up already created resources.
    vkDestroySurfaceKHR(m_context->getVkInstance(), m_vkSurface, nullptr);
    throw;
  }

  m_device->initSwapchain(window->getWidth(), window->getHeight());

  m_triangleVertShader = loadShaderAsset(
      m_device.get(), pal::getCurExecutablePath().parent_path() / "data/shaders/triangle.vert.spv");
  m_triangleFragShader = loadShaderAsset(
      m_device.get(), pal::getCurExecutablePath().parent_path() / "data/shaders/triangle.frag.spv");
}

NativeSurface::~NativeSurface() {
  m_triangleVertShader = nullptr;
  m_triangleFragShader = nullptr;

  m_device = nullptr;
  vkDestroySurfaceKHR(m_context->getVkInstance(), m_vkSurface, nullptr);
}

} // namespace tria::gfx

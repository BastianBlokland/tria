#pragma once
#include "internal/device.hpp"
#include "internal/graphic_manager.hpp"
#include "internal/renderer.hpp"
#include "internal/shader_manager.hpp"
#include "internal/swapchain.hpp"
#include "tria/gfx/context.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <array>
#include <optional>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeContext;

/*
 * Implementation of a graphics 'Canvas' abstraction.
 */
class NativeCanvas final {
public:
  NativeCanvas(
      log::Logger* logger,
      const NativeContext* context,
      const pal::Window* window,
      VSyncMode vSync);
  ~NativeCanvas();

  /* Begin recording draw commands.
   * Returns: false if we failed to begin recordering (for example because the window is minimized).
   */
  [[nodiscard]] auto drawBegin() -> bool;

  /* Record a draw with the given asset.
   */
  auto draw(const asset::Graphic* asset, uint16_t vertexCount) -> void;

  /* Stop recording draw commands, execute the commands and present the result to the surface
   * (window).
   */
  auto drawEnd() -> void;

private:
  log::Logger* m_logger;
  const NativeContext* m_context;
  const pal::Window* m_window;
  internal::DeviceUnique m_device;
  internal::ShaderManagerUnique m_shaderManager;
  internal::GraphicManagerUnique m_graphicManager;
  VkRenderPass m_vkRenderPass;
  internal::SwapchainUnique m_swapchain;

  // Two renderers to support recording one while is other is currently being rendered on the gpu.
  std::array<internal::RendererUnique, 2> m_renderers;
  bool m_frontRenderer; // Are we using renderer 0 or 1 atm to record.

  uint16_t m_lastWinWidth;
  uint16_t m_lastWinHeight;

  std::optional<uint32_t> m_curSwapchainImgIdx;

  [[nodiscard]] auto getCurRenderer() const noexcept -> internal::Renderer& {
    return *m_renderers[static_cast<unsigned int>(m_frontRenderer)];
  }

  [[nodiscard]] auto cycleRenderer() noexcept { m_frontRenderer = !m_frontRenderer; }
};

} // namespace tria::gfx

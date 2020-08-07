#pragma once
#include "internal/asset_resource.hpp"
#include "internal/device.hpp"
#include "internal/graphic.hpp"
#include "internal/mesh.hpp"
#include "internal/renderer.hpp"
#include "internal/shader.hpp"
#include "internal/swapchain.hpp"
#include "internal/texture.hpp"
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
  NativeCanvas(const NativeCanvas& rhs)     = delete;
  NativeCanvas(NativeCanvas&& rhs) noexcept = delete;
  ~NativeCanvas();

  auto operator=(const NativeCanvas& rhs) -> NativeCanvas& = delete;
  auto operator=(NativeCanvas&& rhs) noexcept -> NativeCanvas& = delete;

  /* Begin recording draw commands.
   * Returns: false if we failed to begin recordering (for example because the window is minimized).
   */
  [[nodiscard]] auto drawBegin(math::Color clearCol) -> bool;

  /* Record a draw with the given asset.
   */
  auto draw(const asset::Graphic* asset, const void* uniData, size_t uniSize, uint32_t size)
      -> void;

  /* Stop recording draw commands, execute the commands and present the result to the surface
   * (window).
   */
  auto drawEnd() -> void;

private:
  log::Logger* m_logger;
  const NativeContext* m_context;
  const pal::Window* m_window;
  internal::DeviceUnique m_device;
  internal::AssetResourceUnique<internal::Shader> m_shaders;
  internal::AssetResourceUnique<internal::Mesh> m_meshes;
  internal::AssetResourceUnique<internal::Texture> m_textures;
  internal::AssetResourceUnique<internal::Graphic> m_graphics;
  VkRenderPass m_vkRenderPass;
  internal::SwapchainUnique m_swapchain;

  // Two renderers to support recording one while is other is currently being rendered on the gpu.
  std::array<internal::RendererUnique, 2> m_renderers;
  bool m_frontRenderer; // Are we using renderer 0 or 1 atm to record.

  pal::WindowSize m_lastWinSize;

  std::optional<uint32_t> m_curSwapchainImgIdx;

  [[nodiscard]] auto getCurRenderer() const noexcept -> internal::Renderer& {
    return *m_renderers[static_cast<unsigned int>(m_frontRenderer)];
  }

  [[nodiscard]] auto cycleRenderer() noexcept { m_frontRenderer = !m_frontRenderer; }
};

} // namespace tria::gfx

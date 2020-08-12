#include "native_canvas.hpp"
#include "internal/utils.hpp"
#include "native_context.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "tria/gfx/err/sync_err.hpp"
#include "tria/pal/utils.hpp"
#include <cassert>

namespace tria::gfx {

using namespace internal;

NativeCanvas::NativeCanvas(
    log::Logger* logger, const NativeContext* context, const pal::Window* window, VSyncMode vSync) :
    m_logger{logger},
    m_context{context},
    m_window{window},
    m_frontRenderer{false},
    m_curSwapchainImgIdx{std::nullopt} {
  assert(m_context);
  assert(m_window);

  m_device = getDevice(m_logger, m_context, window);
  if (!m_device) {
    throw err::GfxErr{"No device found with vulkan support"};
  }

  m_shaders  = std::make_unique<AssetResource<Shader>>(m_logger, m_device.get());
  m_meshes   = std::make_unique<AssetResource<Mesh>>(m_logger, m_device.get());
  m_textures = std::make_unique<AssetResource<Texture>>(m_logger, m_device.get());
  m_graphics = std::make_unique<AssetResource<Graphic>>(m_logger, m_device.get());

  m_fwdTechnique = std::make_unique<ForwardTechnique>(m_device.get());

  m_swapchain = std::make_unique<Swapchain>(logger, m_device.get(), vSync);

  for (auto i = 0U; i != m_renderers.size(); ++i) {
    m_renderers[i] = std::make_unique<Renderer>(m_logger, m_device.get());
  }
}

NativeCanvas::~NativeCanvas() {
  // Explicitly set to null to control the destruction order.
  for (auto i = 0U; i != m_renderers.size(); ++i) {
    m_renderers[i] = nullptr;
  }
  m_graphics     = nullptr;
  m_shaders      = nullptr;
  m_meshes       = nullptr;
  m_textures     = nullptr;
  m_swapchain    = nullptr;
  m_fwdTechnique = nullptr;
  m_device       = nullptr;
}

auto NativeCanvas::getDrawStats() const noexcept -> DrawStats {
  const auto isDrawActive = m_curSwapchainImgIdx.has_value();
  const auto& renderer    = isDrawActive ? getPrevRenderer() : getCurRenderer();
  return renderer.getDrawStats();
}

auto NativeCanvas::drawBegin(math::Color clearCol) -> bool {
  if (m_curSwapchainImgIdx) {
    throw err::SyncErr{"Unable to begin a draw: draw already active"};
  }

  // Detect if the window has been resized, if so force recreate the swapchain. This is required as
  // not all drivers report an out-of-date swapchain if the size does not match the window anymore.
  const auto winHasResized = m_window->getSize() != m_lastWinSize;
  if (winHasResized) {
    m_lastWinSize = m_window->getSize();
  }

  // Pick the next renderer (so we can record while the previous is still rendering on the gpu).
  cycleRenderer();
  auto& curRenderer = getCurRenderer();

  // Wait until this renderer is ready to record a new frame.
  curRenderer.waitUntilReady();

  // Acquire an image to render into.
  const auto swapImgIdx = m_swapchain->acquireImage(curRenderer.getImageAvailable(), winHasResized);
  // Failing to acquire an image can be a valid scenario, for example due to a minimized window.
  if (!swapImgIdx) {
    return false;
  }
  m_curSwapchainImgIdx = *swapImgIdx;

  m_fwdTechnique->prepareResources(*m_swapchain);
  curRenderer.drawBegin(*m_fwdTechnique, *swapImgIdx, clearCol);

  return true;
}

auto NativeCanvas::draw(
    const asset::Graphic* asset, const void* instData, size_t instDataSize, uint32_t count)
    -> void {
  if (!m_curSwapchainImgIdx) {
    throw err::SyncErr{"Unable record a draw: no draw active"};
  }

  const auto* graphic = m_graphics->get(asset, m_shaders.get(), m_meshes.get(), m_textures.get());
  getCurRenderer().draw(*m_fwdTechnique, graphic, instData, instDataSize, count);
}

auto NativeCanvas::drawEnd() -> void {
  if (!m_curSwapchainImgIdx) {
    throw err::SyncErr{"Unable to end a draw: no draw active"};
  }

  auto& curRenderer = getCurRenderer();

  // Wait until the previous renderer is finished, reason is that we only want a single frame in
  // flight on the gpu at the same time.
  getPrevRenderer().waitUntilReady();

  curRenderer.drawEnd();

  m_swapchain->presentImage(curRenderer.getImageFinished(), *m_curSwapchainImgIdx);

  m_curSwapchainImgIdx = std::nullopt;
}

} // namespace tria::gfx

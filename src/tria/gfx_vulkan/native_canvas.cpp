#include "native_canvas.hpp"
#include "internal/utils.hpp"
#include "native_context.hpp"
#include "tria/gfx/err/sync_err.hpp"
#include "tria/pal/native.hpp"
#include "tria/pal/utils.hpp"
#include <array>
#include <cassert>
#include <optional>

namespace tria::gfx {

using namespace internal;

namespace {

[[nodiscard]] auto createVkRenderPass(const Device* device) -> VkRenderPass {
  assert(device);

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format                  = device->getVkSurfaceFormat().format;
  colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment            = 0;
  colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorAttachmentRef;

  VkSubpassDependency dependency                  = {};
  dependency.srcSubpass                           = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass                           = 0;
  dependency.srcStageMask                         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask                        = 0;
  dependency.dstStageMask                         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask                        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  std::array<VkSubpassDependency, 1> dependencies = {dependency};

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount        = 1;
  renderPassInfo.pAttachments           = &colorAttachment;
  renderPassInfo.subpassCount           = 1;
  renderPassInfo.pSubpasses             = &subpass;
  renderPassInfo.dependencyCount        = dependencies.size();
  renderPassInfo.pDependencies          = dependencies.data();

  VkRenderPass result;
  checkVkResult(vkCreateRenderPass(device->getVkDevice(), &renderPassInfo, nullptr, &result));
  return result;
}

} // namespace

NativeCanvas::NativeCanvas(
    log::Logger* logger, const NativeContext* context, const pal::Window* window, VSyncMode vSync) :
    m_logger{logger},
    m_context{context},
    m_window{window},
    m_frontRenderer{false},
    m_curSwapchainImgIdx{std::nullopt} {
  assert(m_context);
  assert(m_window);

  m_device = getDevice(m_logger, m_context->getVkInstance(), window);
  if (!m_device) {
    throw err::DriverErr{"No device found with vulkan support"};
  }

  m_shaderManager = std::make_unique<ShaderManager>(m_logger, m_device.get());
  m_graphicManager =
      std::make_unique<GraphicManager>(m_logger, m_device.get(), m_shaderManager.get());

  m_vkRenderPass = createVkRenderPass(m_device.get());

  m_swapchain = std::make_unique<Swapchain>(logger, m_device.get(), vSync);

  for (auto i = 0U; i != m_renderers.size(); ++i) {
    m_renderers[i] = std::make_unique<Renderer>(m_device.get());
  }
}

NativeCanvas::~NativeCanvas() {
  for (auto i = 0U; i != m_renderers.size(); ++i) {
    m_renderers[i] = nullptr;
  }
  m_graphicManager = nullptr;
  m_shaderManager  = nullptr;
  m_swapchain      = nullptr;
  vkDestroyRenderPass(m_device->getVkDevice(), m_vkRenderPass, nullptr);
  m_device = nullptr;
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
  const auto swapImgIdx =
      m_swapchain->acquireImage(m_vkRenderPass, curRenderer.getImageAvailable(), winHasResized);
  // Failing to acquire an image can be a valid scenario, for example due to a minimized window.
  if (!swapImgIdx) {
    return false;
  }
  m_curSwapchainImgIdx = *swapImgIdx;

  curRenderer.drawBegin(
      m_vkRenderPass,
      m_swapchain->getVkFramebuffer(*m_curSwapchainImgIdx),
      m_swapchain->getExtent(),
      clearCol);

  return true;
}

auto NativeCanvas::draw(const asset::Graphic* asset, uint16_t vertexCount) -> void {
  if (!m_curSwapchainImgIdx) {
    throw err::SyncErr{"Unable record a draw: no draw active"};
  }

  const auto& graphic = m_graphicManager->getGraphic(asset, m_vkRenderPass);
  getCurRenderer().draw(graphic, vertexCount);
}

auto NativeCanvas::drawEnd() -> void {
  if (!m_curSwapchainImgIdx) {
    throw err::SyncErr{"Unable to end a draw: no draw active"};
  }

  auto& curRenderer = getCurRenderer();
  curRenderer.drawEnd();
  m_swapchain->presentImage(curRenderer.getImageFinished(), *m_curSwapchainImgIdx);

  m_curSwapchainImgIdx = std::nullopt;
}

} // namespace tria::gfx

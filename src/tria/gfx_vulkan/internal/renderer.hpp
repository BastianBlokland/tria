#pragma once
#include "device.hpp"
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/*
 * Renderer is a handle to submit work to the gpu.
 * Draw commands can be recordered on it and then submitted to the gpu for execution.
 * Note: No new commands can be recorded while its still executing on the gpu, thats why its usefull
 * to have two renderers so you can record one while the other is executing.
 */
class Renderer final {
public:
  Renderer(const Device* device);
  ~Renderer();

  /* The renderer will wait (on the gpu) for this semaphore before starting to render.
   */
  [[nodiscard]] auto getImageAvailable() const noexcept { return m_imgAvailable; }

  /* The renderer will complete this semaphore when its done rendering the image.
   */
  [[nodiscard]] auto getImageFinished() const noexcept { return m_imgFinished; }

  /* Begin recording new draw commands to this renderer.
   * Note: Will block if the renderer is currently still rendering.
   */
  auto drawBegin(VkRenderPass vkRenderPass, VkFramebuffer vkFrameBuffer, VkExtent2D extent) -> void;

  /* Finish recordering draw commands and submit the work to the gpu.
   */
  auto drawEnd() -> void;

private:
  const Device* m_device;
  VkSemaphore m_imgAvailable;
  VkSemaphore m_imgFinished;
  VkFence m_renderDone;
  VkCommandBuffer m_gfxVkCommandBuffer;

  auto waitForDone() -> void;
  auto markNotDone() -> void;
};

using RendererUnique = std::unique_ptr<Renderer>;

} // namespace tria::gfx::internal

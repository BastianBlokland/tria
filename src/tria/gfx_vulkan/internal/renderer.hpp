#pragma once
#include "forward_technique.hpp"
#include "graphic.hpp"
#include "stat_recorder.hpp"
#include "stopwatch.hpp"
#include "transferer.hpp"
#include "tria/gfx/canvas.hpp"
#include "tria/log/api.hpp"
#include "tria/math/vec.hpp"
#include "uniform_container.hpp"
#include <array>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/*
 * Renderer is a handle to submit work to the gpu.
 * Draw commands can be recordered on it and then submitted to the gpu for execution.
 * Note: No new commands can be recorded while its still executing on the gpu, thats why its usefull
 * to have two renderers so you can record one while the other is executing.
 */
class Renderer final {
public:
  Renderer(log::Logger* logger, Device* device);
  Renderer(const Renderer& rhs)     = delete;
  Renderer(Renderer&& rhs) noexcept = delete;
  ~Renderer();

  auto operator=(const Renderer& rhs) -> Renderer& = delete;
  auto operator=(Renderer&& rhs) noexcept -> Renderer& = delete;

  /* Get statistics for the last draw.
   */
  [[nodiscard]] auto getDrawStats() const noexcept -> DrawStats;

  /* The renderer will wait (on the gpu) for this semaphore before starting to render.
   */
  [[nodiscard]] auto getImageAvailable() const noexcept { return m_imgAvailable; }

  /* The renderer will complete this semaphore when its done rendering the image.
   */
  [[nodiscard]] auto getImageFinished() const noexcept { return m_imgFinished; }

  /* Wait until this rendering is ready to record a new frame.
   */
  auto waitUntilReady() const -> void;

  /* Begin recording new draw commands to this renderer.
   * Note: Will block if the renderer is currently still rendering.
   */
  auto drawBegin(const ForwardTechnique& technique, SwapchainIdx swapIdx, math::Color clearCol)
      -> void;

  auto bindGlobalData(const void* data, size_t dataSize) -> void;

  /* Record a draw of the given graphic.
   */
  auto draw(
      const ForwardTechnique& technique,
      const Graphic* graphic,
      uint32_t indexCount,
      const void* uniData,
      size_t uniSize,
      uint32_t count) -> void;

  /* Finish recordering draw commands and submit the work to the gpu.
   */
  auto drawEnd() -> void;

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkSemaphore m_imgAvailable;
  VkSemaphore m_imgFinished;
  VkFence m_renderDone;
  TransfererUnique m_transferer;
  UniformContainerUnique m_uni;
  StopwatchUnique m_stopwatch;
  StatRecorderUnique m_statRecorder;
  VkPipelineLayout m_globalPipelineLayout;

  bool m_hasSubmittedDrawOnce; // Indicates if the renderer has ever submitted a draw.
  bool m_hasBoundGlobalData;
  uint32_t m_drawId;

  TimestampRecord m_drawStart;
  TimestampRecord m_drawEnd;

  /* We record two separate commandbuffers, one for the drawing commands and one for the transfer
   * commands. But at the moment both buffers are submitted to the Graphics queue, in the future we
   * could use the transfer queue also.
   * All draw commands wait for all transfers to be done before executing.
   */
  std::array<VkCommandBuffer, 2> m_gfxVkCommandBuffers;
  const VkCommandBuffer& m_transferVkCommandBuffer = m_gfxVkCommandBuffers[0];
  const VkCommandBuffer& m_drawVkCommandBuffer     = m_gfxVkCommandBuffers[1];

  auto waitForDone() const -> void;
  auto markNotDone() -> void;
};

using RendererUnique = std::unique_ptr<Renderer>;

} // namespace tria::gfx::internal

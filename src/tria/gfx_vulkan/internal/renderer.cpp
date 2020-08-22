#include "renderer.hpp"
#include "../native_context.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include <cassert>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

/* Maximum amount of instances we will ever draw in a single call. Usefull in shaders to have an
 * upper bound on the uniform data arrays.
 */
constexpr auto g_maxInstanceCount = 2048U;

template <unsigned int Count>
[[nodiscard]] auto createGfxVkCommandBuffers(const Device* device)
    -> std::array<VkCommandBuffer, Count> {
  assert(device);

  VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
  commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocInfo.commandPool        = device->getGraphicsVkCommandPool();
  commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocInfo.commandBufferCount = Count;

  std::array<VkCommandBuffer, Count> result;
  checkVkResult(
      vkAllocateCommandBuffers(device->getVkDevice(), &commandBufferAllocInfo, result.data()));
  return result;
}

auto beginCommandBuffer(VkCommandBuffer vkCommandBuffer) -> void {
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  checkVkResult(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));
}

template <unsigned int Count>
auto submitCommandBuffers(
    const Device* device,
    const std::array<VkCommandBuffer, Count>& vkCommandBuffers,
    VkSemaphore beginSemaphore,
    VkPipelineStageFlags beginWaitStage,
    VkSemaphore endSemaphore,
    VkFence endFence) -> void {
  assert(device);

  VkSubmitInfo submitInfo         = {};
  submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount   = 1;
  submitInfo.pWaitSemaphores      = &beginSemaphore;
  submitInfo.pWaitDstStageMask    = &beginWaitStage;
  submitInfo.commandBufferCount   = vkCommandBuffers.size();
  submitInfo.pCommandBuffers      = vkCommandBuffers.data();
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &endSemaphore;
  checkVkResult(vkQueueSubmit(device->getVkGraphicsQueue(), 1, &submitInfo, endFence));
}

/* Memory barrier that waits for transferring to be done before starting any rendering.
 */
auto insertWaitForTransferMemBarrier(VkCommandBuffer vkCommandBuffer) -> void {
  VkMemoryBarrier transferMemBarrier = {};
  transferMemBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  transferMemBarrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
  transferMemBarrier.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;

  vkCmdPipelineBarrier(
      vkCommandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      0,
      1,
      &transferMemBarrier,
      0,
      nullptr,
      0,
      nullptr);
}

auto setViewport(VkCommandBuffer vkCommandBuffer, ImageSize size) -> void {
  VkViewport viewport = {};
  viewport.x          = 0.0f;
  viewport.y          = 0.0f;
  viewport.width      = static_cast<float>(size.x());
  viewport.height     = static_cast<float>(size.y());
  viewport.minDepth   = 0.0f;
  viewport.maxDepth   = 1.0f;
  vkCmdSetViewport(vkCommandBuffer, 0U, 1U, &viewport);
}

auto setScissor(VkCommandBuffer vkCommandBuffer, ImageSize size) -> void {
  VkRect2D scissor      = {};
  scissor.offset        = {0, 0};
  scissor.extent.width  = size.x();
  scissor.extent.height = size.y();
  vkCmdSetScissor(vkCommandBuffer, 0U, 1U, &scissor);
}

} // namespace

Renderer::Renderer(log::Logger* logger, Device* device) :
    m_logger{logger}, m_device{device}, m_hasSubmittedDrawOnce{false}, m_drawId{0U} {
  if (!m_device) {
    throw std::invalid_argument{"Device cannot be null"};
  }

  m_imgAvailable = createVkSemaphore(device->getVkDevice());
  DBG_SEMPAHORE_NAME(m_device, m_imgAvailable, "image_available");

  m_imgFinished = createVkSemaphore(device->getVkDevice());
  DBG_SEMPAHORE_NAME(m_device, m_imgFinished, "image_finished");

  m_renderDone = createVkFence(device->getVkDevice(), true);
  DBG_FENCE_NAME(m_device, m_renderDone, "render_done");

  m_transferer          = std::make_unique<Transferer>(logger, device);
  m_uni                 = std::make_unique<UniformContainer>(logger, device);
  m_stopwatch           = std::make_unique<Stopwatch>(logger, device);
  m_statRecorder        = std::make_unique<StatRecorder>(logger, device);
  m_gfxVkCommandBuffers = createGfxVkCommandBuffers<2>(device);

  DBG_COMMANDBUFFER_NAME(m_device, m_transferVkCommandBuffer, "transfer");
  DBG_COMMANDBUFFER_NAME(m_device, m_drawVkCommandBuffer, "draw");
}

Renderer::~Renderer() {
  try {
    // Wait for this renderer to be done executing on the gpu.
    waitForDone();

    vkFreeCommandBuffers(
        m_device->getVkDevice(),
        m_device->getGraphicsVkCommandPool(),
        m_gfxVkCommandBuffers.size(),
        m_gfxVkCommandBuffers.data());
    m_transferer = nullptr;
    m_uni        = nullptr;
    vkDestroySemaphore(m_device->getVkDevice(), m_imgAvailable, nullptr);
    vkDestroySemaphore(m_device->getVkDevice(), m_imgFinished, nullptr);
    vkDestroyFence(m_device->getVkDevice(), m_renderDone, nullptr);

  } catch (...) {
    LOG_E(m_logger, "Failed to cleanup vulkan renderer");
  }
}

auto Renderer::getDrawStats() const noexcept -> DrawStats {
  if (!m_hasSubmittedDrawOnce) {
    // If we've never submitted a draw then there are no statistics we can collect.
    return {};
  }

  // Wait for this renderer to be done executing on the gpu.
  waitForDone();

  const auto start = m_stopwatch->getTimestamp(m_drawStart);
  const auto end   = m_stopwatch->getTimestamp(m_drawEnd);

  DrawStats result;
  result.gpuTime                 = std::chrono::duration<double, std::nano>(end - start);
  result.inputAssemblyVerts      = m_statRecorder->getStat(StatType::InputAssemblyVerts);
  result.inputAssemblyPrimitives = m_statRecorder->getStat(StatType::InputAssemblyPrimitives);
  result.vertShaderInvocations   = m_statRecorder->getStat(StatType::VertShaderInvocations);
  result.fragShaderInvocations   = m_statRecorder->getStat(StatType::FragShaderInvocations);
  return result;
}

auto Renderer::waitUntilReady() const -> void {
  // Wait for this renderer to be done executing on the gpu.
  waitForDone();
}

auto Renderer::drawBegin(
    const ForwardTechnique& technique, SwapchainIdx swapIdx, math::Color clearCol) -> void {

  // Wait for this renderer to be done executing on the gpu.
  waitForDone();

  // Clear any resources from the last execution.
  m_transferer->reset();
  m_uni->reset();
  m_drawId = 0U;

  beginCommandBuffer(m_drawVkCommandBuffer);
  m_stopwatch->reset(m_drawVkCommandBuffer);
  m_statRecorder->reset(m_drawVkCommandBuffer);

  m_drawStart = m_stopwatch->markTimestamp(m_drawVkCommandBuffer);

  // Wait with rendering until transferring has finished.
  // Transfers are recorded to a separate 'm_transferVkCommandBuffer' buffer and submitted first.
  insertWaitForTransferMemBarrier(m_drawVkCommandBuffer);

  technique.beginRenderPass(m_drawVkCommandBuffer, swapIdx, clearCol);

  setViewport(m_drawVkCommandBuffer, technique.getSize());
  setScissor(m_drawVkCommandBuffer, technique.getSize());

  m_statRecorder->beginCapture(m_drawVkCommandBuffer);
}

auto Renderer::draw(
    const ForwardTechnique& technique,
    const Graphic* graphic,
    uint32_t indexCount,
    const void* instData,
    const size_t instDataSize,
    uint32_t count) -> void {

  if (instDataSize > m_uni->getMaxDataSize()) {
    LOG_W(m_logger, "Instance data size exceeds maximum", {"graphic", graphic->getId()});
    return;
  }

  DBG_CMD_BEGIN_LABEL(
      m_device, m_drawVkCommandBuffer, "Draw " + graphic->getId(), math::color::get(m_drawId));

  // Prepare and bind per graphic resources (pipeline and index buffer).
  graphic->prepareResources(
      m_transferer.get(), m_uni.get(), technique.getVkRenderPass(), technique.getSampleCount());
  vkCmdBindPipeline(
      m_drawVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphic->getVkPipeline());

  const auto* mesh = graphic->getMesh();
  if (mesh) {
    vkCmdBindIndexBuffer(
        m_drawVkCommandBuffer,
        mesh->getIndexBuffer().getVkBuffer(),
        0U,
        getVkIndexType<Mesh::IndexType>());
    if (!indexCount) {
      // Zero indexCount indicates we should draw all indices.
      indexCount = mesh->getIndexCount();
    } else {
      // Otherwise draw up to the indexCount.
      indexCount = std::min(static_cast<size_t>(indexCount), mesh->getIndexCount());
    }
  }

  // Bind the descriptor set from the graphic itself (containing for example the mesh and textures).
  const auto set = graphic->getVkDescriptorSet();
  vkCmdBindDescriptorSets(
      m_drawVkCommandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      graphic->getVkPipelineLayout(),
      g_shaderResourceGraphicSetId,
      1U,
      &set,
      0U,
      nullptr);

  if (!indexCount) {
    DBG_CMD_END_LABEL(m_device, m_drawVkCommandBuffer);
    LOG_W(
        m_logger,
        "IndexCount of zero is provided but graphic has no mesh",
        {"graphic", graphic->getId()});
    return;
  }

  if (graphic->getUsesInstanceData() && (!instData || instDataSize == 0U)) {
    DBG_CMD_END_LABEL(m_device, m_drawVkCommandBuffer);
    LOG_W(
        m_logger,
        "Graphic uses instance data but none was provided",
        {"graphic", graphic->getId()});
    return;
  }

  // Submit the draws in batches.
  while (count > 0U) {
    // Calculate how many instances we can draw in a call.
    const auto instanceCount = std::min(
        instDataSize == 0U
            ? count
            : std::min(count, m_uni->getMaxDataSize() / static_cast<uint32_t>(instDataSize)),
        g_maxInstanceCount);

    // Bind the instance uniform data.
    if (graphic->getUsesInstanceData()) {
      auto [uniDescSet, uniOffset] = m_uni->upload(instData, instDataSize * instanceCount);
      vkCmdBindDescriptorSets(
          m_drawVkCommandBuffer,
          VK_PIPELINE_BIND_POINT_GRAPHICS,
          graphic->getVkPipelineLayout(),
          g_shaderResourceInstanceSetId,
          1U,
          &uniDescSet,
          1U,
          &uniOffset);
    }

    if (mesh) {
      vkCmdDrawIndexed(m_drawVkCommandBuffer, indexCount, instanceCount, 0U, 0, 0U);
    } else {
      vkCmdDraw(m_drawVkCommandBuffer, indexCount, instanceCount, 0U, 0U);
    }

    // Advance count and the uniform-data pointer to the next batch.
    count -= instanceCount;
    instData = static_cast<const uint8_t*>(instData) + instanceCount * instDataSize;
  }

  DBG_CMD_END_LABEL(m_device, m_drawVkCommandBuffer);
  ++m_drawId;
}

auto Renderer::drawEnd() -> void {

  m_statRecorder->endCapture(m_drawVkCommandBuffer);

  vkCmdEndRenderPass(m_drawVkCommandBuffer);
  m_drawEnd = m_stopwatch->markTimestamp(m_drawVkCommandBuffer);

  vkEndCommandBuffer(m_drawVkCommandBuffer);

  // Record all data transfer needed for this frame.
  beginCommandBuffer(m_transferVkCommandBuffer);
  m_transferer->record(m_transferVkCommandBuffer);
  vkEndCommandBuffer(m_transferVkCommandBuffer);

  markNotDone();
  submitCommandBuffers<2>(
      m_device,
      m_gfxVkCommandBuffers,
      m_imgAvailable,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      m_imgFinished,
      m_renderDone);
  m_hasSubmittedDrawOnce = true;
}

auto Renderer::waitForDone() const -> void {
  checkVkResult(vkWaitForFences(m_device->getVkDevice(), 1, &m_renderDone, true, UINT64_MAX));
}

auto Renderer::markNotDone() -> void {
  checkVkResult(vkResetFences(m_device->getVkDevice(), 1, &m_renderDone));
}

} // namespace tria::gfx::internal

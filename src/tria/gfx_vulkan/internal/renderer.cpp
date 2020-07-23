#include "renderer.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createGfxVkCommandBuffer(const Device* device) -> VkCommandBuffer {
  assert(device);

  VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
  commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocInfo.commandPool        = device->getGraphicsVkCommandPool();
  commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocInfo.commandBufferCount = 1;

  VkCommandBuffer result;
  checkVkResult(vkAllocateCommandBuffers(device->getVkDevice(), &commandBufferAllocInfo, &result));
  return result;
}

auto beginCommandBuffer(VkCommandBuffer vkCommandBuffer) -> void {
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  checkVkResult(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));
}

auto submitCommandBuffer(
    const Device* device,
    VkCommandBuffer vkCommandBuffer,
    VkSemaphore beginSemaphore,
    VkSemaphore endSemaphore,
    VkFence endFence) -> void {
  assert(device);

  std::array<VkSemaphore, 1> waitSemaphores      = {beginSemaphore};
  std::array<VkSemaphore, 1> signalSemaphores    = {endSemaphore};
  std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo         = {};
  submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount   = waitSemaphores.size();
  submitInfo.pWaitSemaphores      = waitSemaphores.data();
  submitInfo.pWaitDstStageMask    = waitStages.data();
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = &vkCommandBuffer;
  submitInfo.signalSemaphoreCount = signalSemaphores.size();
  submitInfo.pSignalSemaphores    = signalSemaphores.data();
  checkVkResult(vkQueueSubmit(device->getVkGraphicsQueue(), 1, &submitInfo, endFence));
}

auto beginRenderPass(
    VkCommandBuffer vkCommandBuffer,
    VkRenderPass vkRenderPass,
    VkFramebuffer vkFramebuffer,
    VkExtent2D extent,
    math::Color clearCol) -> void {

  static_assert(clearCol.getSize() == 4);
  VkClearColorValue clearColorValue;
  clearCol.memcpy(clearColorValue.float32);

  std::array<VkClearValue, 1> clearValues = {VkClearValue{clearColorValue}};

  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass            = vkRenderPass;
  renderPassInfo.framebuffer           = vkFramebuffer;
  renderPassInfo.renderArea.offset     = {0, 0};
  renderPassInfo.renderArea.extent     = extent;
  renderPassInfo.clearValueCount       = clearValues.size();
  renderPassInfo.pClearValues          = clearValues.data();

  vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

auto setViewport(VkCommandBuffer vkCommandBuffer, VkExtent2D extent) -> void {
  VkViewport viewport = {};
  viewport.x          = 0.0f;
  viewport.y          = 0.0f;
  viewport.width      = static_cast<float>(extent.width);
  viewport.height     = static_cast<float>(extent.height);
  viewport.minDepth   = 0.0f;
  viewport.maxDepth   = 1.0f;
  vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
}

auto setScissor(VkCommandBuffer vkCommandBuffer, VkExtent2D extent) -> void {
  VkRect2D scissor = {};
  scissor.offset   = {0, 0};
  scissor.extent   = extent;
  vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
}

} // namespace

Renderer::Renderer(const Device* device) : m_device{device} {
  if (!m_device) {
    throw std::invalid_argument{"Device cannot be null"};
  }

  m_imgAvailable       = createVkSemaphore(device->getVkDevice());
  m_imgFinished        = createVkSemaphore(device->getVkDevice());
  m_renderDone         = createVkFence(device->getVkDevice(), true);
  m_gfxVkCommandBuffer = createGfxVkCommandBuffer(device);
}

Renderer::~Renderer() {
  // Wait for this renderer to be done executing on the gpu.
  waitForDone();

  vkFreeCommandBuffers(
      m_device->getVkDevice(), m_device->getGraphicsVkCommandPool(), 1, &m_gfxVkCommandBuffer);
  vkDestroySemaphore(m_device->getVkDevice(), m_imgAvailable, nullptr);
  vkDestroySemaphore(m_device->getVkDevice(), m_imgFinished, nullptr);
  vkDestroyFence(m_device->getVkDevice(), m_renderDone, nullptr);
}

auto Renderer::waitUntilReady() -> void {
  // Wait for this renderer to be done executing on the gpu.
  waitForDone();
}

auto Renderer::drawBegin(
    VkRenderPass vkRenderPass, VkFramebuffer vkFrameBuffer, VkExtent2D extent, math::Color clearCol)
    -> void {

  // Wait for this renderer to be done executing on the gpu.
  waitForDone();

  beginCommandBuffer(m_gfxVkCommandBuffer);
  beginRenderPass(m_gfxVkCommandBuffer, vkRenderPass, vkFrameBuffer, extent, clearCol);

  setViewport(m_gfxVkCommandBuffer, extent);
  setScissor(m_gfxVkCommandBuffer, extent);
}

auto Renderer::draw(const Graphic* graphic) -> void {
  const auto* mesh = graphic->getMesh();

  vkCmdBindPipeline(
      m_gfxVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphic->getVkPipeline());

  std::array<VkBuffer, 1> vertexBuffers           = {mesh->getVertexBuffer().getVkBuffer()};
  std::array<VkDeviceSize, 1> vertexBufferOffsets = {0};
  vkCmdBindVertexBuffers(
      m_gfxVkCommandBuffer,
      0,
      vertexBuffers.size(),
      vertexBuffers.data(),
      vertexBufferOffsets.data());

  vkCmdDraw(m_gfxVkCommandBuffer, mesh->getVertexCount(), 1, 0, 0);
}

auto Renderer::drawEnd() -> void {

  vkCmdEndRenderPass(m_gfxVkCommandBuffer);
  vkEndCommandBuffer(m_gfxVkCommandBuffer);

  markNotDone();
  submitCommandBuffer(m_device, m_gfxVkCommandBuffer, m_imgAvailable, m_imgFinished, m_renderDone);
}

auto Renderer::waitForDone() -> void {
  checkVkResult(vkWaitForFences(m_device->getVkDevice(), 1, &m_renderDone, true, UINT64_MAX));
}

auto Renderer::markNotDone() -> void {
  checkVkResult(vkResetFences(m_device->getVkDevice(), 1, &m_renderDone));
}

} // namespace tria::gfx::internal

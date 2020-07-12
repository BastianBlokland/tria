#include "renderer.hpp"
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
    VkExtent2D extent) -> void {

  VkClearColorValue clearColorValue;
  clearColorValue.float32[0]              = 1.0f;
  clearColorValue.float32[1]              = 1.0f;
  clearColorValue.float32[2]              = 1.0f;
  clearColorValue.float32[3]              = 1.0f;
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

auto Renderer::drawBegin(VkRenderPass vkRenderPass, VkFramebuffer vkFrameBuffer, VkExtent2D extent)
    -> void {

  // Wait for this renderer to be done executing on the gpu.
  waitForDone();

  beginCommandBuffer(m_gfxVkCommandBuffer);
  beginRenderPass(m_gfxVkCommandBuffer, vkRenderPass, vkFrameBuffer, extent);
}

auto Renderer::drawEnd() -> void {

  vkCmdEndRenderPass(m_gfxVkCommandBuffer);
  vkEndCommandBuffer(m_gfxVkCommandBuffer);

  markNotDone();
  submitCommandBuffer(m_device, m_gfxVkCommandBuffer, m_imgAvailable, m_imgFinished, m_renderDone);
}

auto Renderer::waitForDone() -> void {
  std::array<VkFence, 1> fences = {m_renderDone};
  vkWaitForFences(m_device->getVkDevice(), fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
}

auto Renderer::markNotDone() -> void {
  std::array<VkFence, 1> fences = {m_renderDone};
  vkResetFences(m_device->getVkDevice(), fences.size(), fences.data());
}

} // namespace tria::gfx::internal

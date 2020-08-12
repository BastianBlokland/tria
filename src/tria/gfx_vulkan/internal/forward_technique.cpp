#include "forward_technique.hpp"
#include "../native_context.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkRenderPass(const Device* device, ClearMask clear) -> VkRenderPass {
  assert(device);

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format                  = device->getVkSurfaceFormat().format;
  colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = (clear & clearMask(Clear::Color)) ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                                             : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  std::array<VkAttachmentDescription, 1> attachments = {
      colorAttachment,
  };

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment            = 0;
  colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask       = 0;
  dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array<VkSubpassDependency, 1> dependencies = {
      dependency,
  };

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount        = attachments.size();
  renderPassInfo.pAttachments           = attachments.data();
  renderPassInfo.subpassCount           = 1;
  renderPassInfo.pSubpasses             = &subpass;
  renderPassInfo.dependencyCount        = dependencies.size();
  renderPassInfo.pDependencies          = dependencies.data();

  VkRenderPass result;
  checkVkResult(vkCreateRenderPass(device->getVkDevice(), &renderPassInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto
createVkFramebuffer(const Device* device, VkRenderPass vkRenderPass, const Image& colorAttachment) {
  assert(device);

  std::array<VkImageView, 1> attachments = {
      colorAttachment.getVkImageView(),
  };
  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass              = vkRenderPass;
  framebufferInfo.attachmentCount         = attachments.size();
  framebufferInfo.pAttachments            = attachments.data();
  framebufferInfo.width                   = colorAttachment.getSize().x();
  framebufferInfo.height                  = colorAttachment.getSize().y();
  framebufferInfo.layers                  = 1;

  VkFramebuffer result;
  checkVkResult(vkCreateFramebuffer(device->getVkDevice(), &framebufferInfo, nullptr, &result));
  return result;
}

auto beginVkRenderPass(
    VkCommandBuffer vkCommandBuffer,
    VkRenderPass vkRenderPass,
    VkFramebuffer vkFramebuffer,
    ImageSize renderSize,
    math::Color clearCol) -> void {

  static_assert(clearCol.getSize() == 4);
  VkClearColorValue clearColorValue;
  clearCol.memcpy(clearColorValue.float32);

  std::array<VkClearValue, 1> clearValues = {
      VkClearValue{clearColorValue},
  };

  VkRenderPassBeginInfo renderPassInfo    = {};
  renderPassInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass               = vkRenderPass;
  renderPassInfo.framebuffer              = vkFramebuffer;
  renderPassInfo.renderArea.offset        = {0, 0};
  renderPassInfo.renderArea.extent.width  = renderSize.x();
  renderPassInfo.renderArea.extent.height = renderSize.y();
  renderPassInfo.clearValueCount          = clearValues.size();
  renderPassInfo.pClearValues             = clearValues.data();

  vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

} // namespace

ForwardTechnique::ForwardTechnique(Device* device, ClearMask clear) : m_device{device} {
  assert(device);
  m_vkRenderPass = createVkRenderPass(m_device, clear);
}

ForwardTechnique::~ForwardTechnique() {
  vkDestroyRenderPass(m_device->getVkDevice(), m_vkRenderPass, nullptr);
  for (const VkFramebuffer& vkFramebuffer : m_vkFramebuffers) {
    vkDestroyFramebuffer(m_device->getVkDevice(), vkFramebuffer, nullptr);
  }
}

auto ForwardTechnique::prepareResources(const Swapchain& swapchain) -> void {
  assert(swapchain.getImageSize().x() > 0);
  assert(swapchain.getImageSize().y() > 0);

  if (m_swapVersion != swapchain.getVersion() || m_vkFramebuffers.empty()) {
    assert(swapchain.getImageCount() > 0);

    // Destroy the previous framebuffers.
    for (const VkFramebuffer& vkFramebuffer : m_vkFramebuffers) {
      vkDestroyFramebuffer(m_device->getVkDevice(), vkFramebuffer, nullptr);
    }
    m_vkFramebuffers.clear();

    // Create new framebuffers.
    for (auto i = 0U; i != swapchain.getImageCount(); ++i) {
      const auto& swapImg = swapchain.getImage(i);
      m_vkFramebuffers.push_back(createVkFramebuffer(m_device, m_vkRenderPass, swapImg));
    }

    m_size        = swapchain.getImageSize();
    m_swapVersion = swapchain.getVersion();
  }
}

auto ForwardTechnique::beginRenderPass(
    VkCommandBuffer vkCommandBuffer, SwapchainIdx idx, math::Color clearCol) const noexcept
    -> void {
  assert(vkCommandBuffer);
  assert(idx < m_vkFramebuffers.size());
  beginVkRenderPass(vkCommandBuffer, m_vkRenderPass, m_vkFramebuffers[idx], m_size, clearCol);
}

} // namespace tria::gfx::internal

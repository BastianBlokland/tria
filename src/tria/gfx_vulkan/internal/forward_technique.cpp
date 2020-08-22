#include "forward_technique.hpp"
#include "../native_context.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include <cassert>
#include <stdexcept>

namespace tria::gfx::internal {

namespace {

constexpr auto g_maxAttachCnt = 3U;

[[nodiscard]] auto createVkRenderPass(
    const Device* device, VkSampleCount sampleCount, DepthMode depth, ClearMask clear)
    -> VkRenderPass {
  assert(device);

  std::array<VkAttachmentDescription, g_maxAttachCnt> attachments;
  auto attachCnt = 0U;
  std::array<VkAttachmentReference, g_maxAttachCnt> colAttachRefs;
  auto colAttachRefCnt = 0U;
  VkAttachmentReference resolveAttachmentRef;
  VkAttachmentReference depthAttachmentRef;

  // Color target.
  {
    attachments[attachCnt]         = {};
    attachments[attachCnt].format  = device->getSurfaceFormat();
    attachments[attachCnt].samples = sampleCount;
    attachments[attachCnt].loadOp  = (clear & clearMask(Clear::Color))
        ? VK_ATTACHMENT_LOAD_OP_CLEAR
        : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[attachCnt].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[attachCnt].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[attachCnt].finalLayout    = sampleCount != VK_SAMPLE_COUNT_1_BIT
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    colAttachRefs[colAttachRefCnt]            = {};
    colAttachRefs[colAttachRefCnt].attachment = attachCnt;
    colAttachRefs[colAttachRefCnt].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ++attachCnt;
    ++colAttachRefCnt;
  }

  // Resolve target (When using multiple samples the result is 'resolved' into this attachment).
  if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
    attachments[attachCnt]                = {};
    attachments[attachCnt].format         = device->getSurfaceFormat();
    attachments[attachCnt].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[attachCnt].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[attachCnt].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[attachCnt].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[attachCnt].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    resolveAttachmentRef            = {};
    resolveAttachmentRef.attachment = attachCnt;
    resolveAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ++attachCnt;
  }

  // Depth target.
  if (depth == DepthMode::Enable) {
    attachments[attachCnt]         = {};
    attachments[attachCnt].format  = device->getDepthFormat();
    attachments[attachCnt].samples = sampleCount;
    attachments[attachCnt].loadOp  = (clear & clearMask(Clear::Depth))
        ? VK_ATTACHMENT_LOAD_OP_CLEAR
        : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[attachCnt].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[attachCnt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[attachCnt].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[attachCnt].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthAttachmentRef            = {};
    depthAttachmentRef.attachment = attachCnt;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    ++attachCnt;
  }

  VkSubpassDescription subpass    = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount    = colAttachRefCnt;
  subpass.pColorAttachments       = colAttachRefs.data();
  subpass.pDepthStencilAttachment = depth == DepthMode::Enable ? &depthAttachmentRef : nullptr;
  subpass.pResolveAttachments =
      sampleCount != VK_SAMPLE_COUNT_1_BIT ? &resolveAttachmentRef : nullptr;

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
  renderPassInfo.attachmentCount        = attachCnt;
  renderPassInfo.pAttachments           = attachments.data();
  renderPassInfo.subpassCount           = 1;
  renderPassInfo.pSubpasses             = &subpass;
  renderPassInfo.dependencyCount        = dependencies.size();
  renderPassInfo.pDependencies          = dependencies.data();

  VkRenderPass result;
  checkVkResult(vkCreateRenderPass(device->getVkDevice(), &renderPassInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkFramebuffer(
    const Device* device,
    VkRenderPass vkRenderPass,
    SwapchainSize size,
    const VkImageView* attachments,
    size_t attachmentCount) {
  assert(device);
  assert(attachmentCount > 0);

  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass              = vkRenderPass;
  framebufferInfo.attachmentCount         = attachmentCount;
  framebufferInfo.pAttachments            = attachments;
  framebufferInfo.width                   = size.x();
  framebufferInfo.height                  = size.y();
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

  std::array<VkClearValue, 2> clearValues;
  clearCol.memcpy(clearValues[0].color.float32);
  // Clear depth to zero, reason is we are using a reversed-z depthbuffer.
  clearValues[1].depthStencil = {0.0f, 0U}; // Depth to zero (stencil is not used).

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

ForwardTechnique::ForwardTechnique(
    Device* device, VkSampleCount sampleCount, DepthMode depth, ClearMask clear) :
    m_device{device}, m_sampleCount{sampleCount}, m_depth{depth} {
  assert(device);
  m_vkRenderPass = createVkRenderPass(m_device, sampleCount, depth, clear);
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

    // Destroy the previous targets.
    m_depthTarget.~Image();
    m_colorTarget.~Image();

    // Destroy the previous framebuffers.
    for (const VkFramebuffer& vkFramebuffer : m_vkFramebuffers) {
      vkDestroyFramebuffer(m_device->getVkDevice(), vkFramebuffer, nullptr);
    }
    m_vkFramebuffers.clear();

    m_size        = swapchain.getImageSize();
    m_swapVersion = swapchain.getVersion();

    // Create a new color target.
    // When not using multi-sampling we just render into the swapchain directly.
    if (m_sampleCount != VK_SAMPLE_COUNT_1_BIT) {
      m_colorTarget = Image{m_device,
                            swapchain.getImageSize(),
                            m_device->getSurfaceFormat(),
                            ImageType::ColorAttachment,
                            m_sampleCount,
                            ImageMipMode::None};
      DBG_IMG_NAME(m_device, m_colorTarget.getVkImage(), "color");
      DBG_IMGVIEW_NAME(m_device, m_colorTarget.getVkImageView(), "color");
    }

    // Create a new depth target.
    if (m_depth == DepthMode::Enable) {
      m_depthTarget = Image{m_device,
                            swapchain.getImageSize(),
                            m_device->getDepthFormat(),
                            ImageType::DepthAttachment,
                            m_sampleCount,
                            ImageMipMode::None};
      DBG_IMG_NAME(m_device, m_depthTarget.getVkImage(), "depth");
      DBG_IMGVIEW_NAME(m_device, m_depthTarget.getVkImageView(), "depth");
    }

    // Create new framebuffers.
    for (auto i = 0U; i != swapchain.getImageCount(); ++i) {
      const auto& swapImg = swapchain.getImage(i);

      std::array<VkImageView, g_maxAttachCnt> attachments;
      auto attachCnt = 0U;

      if (m_sampleCount == VK_SAMPLE_COUNT_1_BIT) {
        // Render directly into the swapchain.
        attachments[attachCnt++] = swapImg.getVkImageView();
      } else {
        // Render with multiple samples into a separate target and then resolve into the swapchain.
        attachments[attachCnt++] = m_colorTarget.getVkImageView();
        attachments[attachCnt++] = swapImg.getVkImageView();
      }
      if (m_depth == DepthMode::Enable) {
        attachments[attachCnt++] = m_depthTarget.getVkImageView();
      }

      m_vkFramebuffers.push_back(
          createVkFramebuffer(m_device, m_vkRenderPass, m_size, attachments.data(), attachCnt));
      DBG_FRAMEBUFFER_NAME(m_device, m_vkFramebuffers.back(), "forward" + std::to_string(i));
    }
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

#include "transferer.hpp"
#include "debug_utils.hpp"
#include "utils.hpp"

namespace tria::gfx::internal {

namespace {

constexpr auto g_minTransferBufferSize = 8U * 1024U * 1024U;

auto recordImageLayoutTransition(
    VkCommandBuffer buffer,
    const Image& img,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkAccessFlags srcAccess,
    VkAccessFlags dstAccess,
    VkPipelineStageFlags srcStageFlags,
    VkPipelineStageFlags dstStageFlags,
    uint32_t baseMipLevel,
    uint32_t mipLevels) -> void {

  VkImageMemoryBarrier barrier            = {};
  barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout                       = oldLayout;
  barrier.newLayout                       = newLayout;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = img.getVkImage();
  barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = baseMipLevel;
  barrier.subresourceRange.levelCount     = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0U;
  barrier.subresourceRange.layerCount     = 1U;
  barrier.srcAccessMask                   = srcAccess;
  barrier.dstAccessMask                   = dstAccess;

  vkCmdPipelineBarrier(
      buffer, srcStageFlags, dstStageFlags, 0U, 0U, nullptr, 0U, nullptr, 1U, &barrier);
}

auto imgLayoutFromUndefToTransferDst(
    VkCommandBuffer buffer, const Image& img, uint32_t baseMipLevel, uint32_t mipLevels) -> void {
  VkImageLayout oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout newLayout            = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  VkAccessFlags srcAccess            = 0U;
  VkAccessFlags dstAccess            = VK_ACCESS_TRANSFER_WRITE_BIT;
  VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  recordImageLayoutTransition(
      buffer,
      img,
      oldLayout,
      newLayout,
      srcAccess,
      dstAccess,
      srcStageFlags,
      dstStageFlags,
      baseMipLevel,
      mipLevels);
}

auto imgLayoutFromTransferDstToTransferSrc(
    VkCommandBuffer buffer, const Image& img, uint32_t baseMipLevel, uint32_t mipLevels) -> void {
  VkImageLayout oldLayout            = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  VkImageLayout newLayout            = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  VkAccessFlags srcAccess            = VK_ACCESS_TRANSFER_WRITE_BIT;
  VkAccessFlags dstAccess            = VK_ACCESS_TRANSFER_READ_BIT;
  VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  recordImageLayoutTransition(
      buffer,
      img,
      oldLayout,
      newLayout,
      srcAccess,
      dstAccess,
      srcStageFlags,
      dstStageFlags,
      baseMipLevel,
      mipLevels);
}

auto imgLayoutFromTransferSrcToShaderRead(
    VkCommandBuffer buffer, const Image& img, uint32_t baseMipLevel, uint32_t mipLevels) -> void {
  VkImageLayout oldLayout            = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  VkImageLayout newLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkAccessFlags srcAccess            = VK_ACCESS_TRANSFER_READ_BIT;
  VkAccessFlags dstAccess            = VK_ACCESS_SHADER_READ_BIT;
  VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkPipelineStageFlags dstStageFlags =
      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  recordImageLayoutTransition(
      buffer,
      img,
      oldLayout,
      newLayout,
      srcAccess,
      dstAccess,
      srcStageFlags,
      dstStageFlags,
      baseMipLevel,
      mipLevels);
}

auto imgLayoutFromTransferDstToShaderRead(
    VkCommandBuffer buffer, const Image& img, uint32_t baseMipLevel, uint32_t mipLevels) -> void {
  VkImageLayout oldLayout            = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  VkImageLayout newLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkAccessFlags srcAccess            = VK_ACCESS_TRANSFER_WRITE_BIT;
  VkAccessFlags dstAccess            = VK_ACCESS_SHADER_READ_BIT;
  VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkPipelineStageFlags dstStageFlags =
      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  recordImageLayoutTransition(
      buffer,
      img,
      oldLayout,
      newLayout,
      srcAccess,
      dstAccess,
      srcStageFlags,
      dstStageFlags,
      baseMipLevel,
      mipLevels);
}

} // namespace

auto Transferer::reset() noexcept -> void {
  m_bufferWork.clear();
  m_imageWork.clear();

  // Reset the taken offset on all transfer buffers.
  for (auto& buffAndOffset : m_transferBuffers) {
    buffAndOffset.second = 0;
  }
}

auto Transferer::queueTransfer(const void* data, const Buffer& dst, size_t dstOffset, size_t size)
    -> void {

  assert(size + dstOffset <= dst.getSize());
  assert(dst.getLocation() == MemoryLocation::Device);

  // Upload the data to a transfer buffer.
  const auto reqAlignment = m_device->getLimits().optimalBufferCopyOffsetAlignment;
  const auto src          = getTransferSpace(size, reqAlignment);
  assert(src.second + size < src.first.getSize());
  src.first.upload(data, size, src.second);

  // Add a work item to copy the data from the transfer buffer to the destination.
  m_bufferWork.push_back(BufferWork{src, dst, dstOffset, size});
}

auto Transferer::queueTransfer(const void* data, const Image& dst) -> void {

  assert(dst.getMipLevels() > 0U);

  // Upload the data to a transfer buffer.
  const auto size         = dst.getDataSize();
  const auto reqAlignment = std::max<size_t>(
      getVkFormatSize(dst.getVkFormat()), m_device->getLimits().optimalBufferCopyOffsetAlignment);
  const auto src = getTransferSpace(size, reqAlignment);
  assert(src.second + size <= src.first.getSize());
  src.first.upload(data, size, src.second);

  // Add a work item to copy the data from the transfer buffer to the destination.
  m_imageWork.push_back(ImageWork{src, dst});
}

auto Transferer::record(VkCommandBuffer buffer) noexcept -> void {
  // Record buffer transfers.
  for (const auto& work : m_bufferWork) {
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = work.src.second;
    copyRegion.dstOffset    = work.dstOffset;
    copyRegion.size         = work.size;
    vkCmdCopyBuffer(buffer, work.src.first.getVkBuffer(), work.dst.getVkBuffer(), 1U, &copyRegion);
  }
  m_bufferWork.clear();

  // Record image transfers.
  // TODO(bastian): We can do the layout changes in batches and use a single PipelineBarrier to
  // transition all of them instead of using a PipelineBarrier per image.
  for (const auto& work : m_imageWork) {
    const auto& img = work.dst;
    imgLayoutFromUndefToTransferDst(buffer, img, 0U, img.getMipLevels());

    // Copy the new data to the image (at mip-level 0).
    VkBufferImageCopy region           = {};
    region.bufferOffset                = work.src.second;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1U;
    region.imageOffset                 = {0U, 0U, 0U};
    region.imageExtent.width           = static_cast<uint32_t>(img.getSize().x());
    region.imageExtent.height          = static_cast<uint32_t>(img.getSize().y());
    region.imageExtent.depth           = 1U;
    vkCmdCopyBufferToImage(
        buffer,
        work.src.first.getVkBuffer(),
        img.getVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1U,
        &region);

    switch (img.getMipMode()) {
    case ImageMipMode::Generate:
      /* Generate the mipmap levels by copying from the previous level at half the size until all
       * levels have been generated. */

      imgLayoutFromTransferDstToTransferSrc(buffer, img, 0U, 1U);

      for (auto i = 1U; i != img.getMipLevels(); ++i) {
        // Blit from the previous mip-level.
        VkImageBlit region               = {};
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.mipLevel   = i - 1U;
        region.srcSubresource.layerCount = 1U;
        region.srcOffsets[1].x           = std::max(img.getSize().x() >> (i - 1U), 1);
        region.srcOffsets[1].y           = std::max(img.getSize().y() >> (i - 1U), 1);
        region.srcOffsets[1].z           = 1U;
        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.mipLevel   = i;
        region.dstSubresource.layerCount = 1U;
        region.dstOffsets[1].x           = std::max(img.getSize().x() >> i, 1);
        region.dstOffsets[1].y           = std::max(img.getSize().y() >> i, 1);
        region.dstOffsets[1].z           = 1U;
        vkCmdBlitImage(
            buffer,
            img.getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            img.getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &region,
            VK_FILTER_LINEAR);

        // Transition the previous mip-level to shader-read (we no longer need to copy from it).
        imgLayoutFromTransferSrcToShaderRead(buffer, img, i - 1U, 1U);

        if (i + 1 < img.getMipLevels()) {
          // Not the last mip-level: transition it to transfer source layout.
          imgLayoutFromTransferDstToTransferSrc(buffer, img, i, 1U);
        } else {
          // Last mip level: transition it to shader-read layout.
          imgLayoutFromTransferDstToShaderRead(buffer, img, i, 1U);
        }
      }

      break;
    default:
      imgLayoutFromTransferDstToShaderRead(buffer, img, 0U, img.getMipLevels());
    }
  }
  m_imageWork.clear();
}

auto Transferer::getTransferSpace(size_t size, size_t alignment) -> std::pair<Buffer&, uint32_t> {
  // Find space in an existing transfer buffer.
  for (auto& [buff, offset] : m_transferBuffers) {
    const auto alignPadding = padToAlignment(offset, alignment);

    // Check if this transfer buffer still has enough space left.
    if (buff.getSize() - offset >= size + alignPadding) {
      const auto resultOffset = offset + alignPadding;
      offset += size + alignPadding;
      return {buff, resultOffset};
    }
  }

  // If no transfer buffers has enough space then create a new one.
  const auto newBuffSize = size > g_minTransferBufferSize ? size : g_minTransferBufferSize;
  auto newBuffer = Buffer{m_device, newBuffSize, MemoryLocation::Host, BufferUsage::HostTransfer};
  DBG_BUFFER_NAME(m_device, newBuffer.getVkBuffer(), "transferer");

  m_transferBuffers.emplace_front(std::move(newBuffer), static_cast<uint32_t>(size));

  LOG_D(m_logger, "Vulkan transfer buffer created", {"size", log::MemSize{newBuffSize}});

  return {m_transferBuffers.front().first, 0U};
}

} // namespace tria::gfx::internal

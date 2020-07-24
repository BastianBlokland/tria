#include "transferer.hpp"
#include "utils.hpp"

namespace tria::gfx::internal {

namespace {

constexpr auto g_minTransferBufferSize = 8 * 1024 * 1024;

} // namespace

auto Transferer::reset() noexcept -> void {
  m_work.clear();

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
  const auto src = getTransferSpace(size);
  assert(src.second + size < src.first.getSize());
  src.first.upload(data, size, src.second);

  // Add a work item to copy the data from the transfer buffer to the destination.
  m_work.push_back(Work{src, dst, dstOffset, size});
}

auto Transferer::record(VkCommandBuffer buffer) noexcept -> void {
  for (const auto& work : m_work) {
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = work.src.second;
    copyRegion.dstOffset    = work.dstOffset;
    copyRegion.size         = work.size;
    vkCmdCopyBuffer(buffer, work.src.first.getVkBuffer(), work.dst.getVkBuffer(), 1, &copyRegion);
  }
  m_work.clear();
}

auto Transferer::getTransferSpace(size_t size) -> std::pair<Buffer&, uint32_t> {
  // Find space in an existing transfer buffer.
  for (auto& [buff, offset] : m_transferBuffers) {
    // Check if this transfer buffer still has enough space left.
    if (buff.getSize() - offset >= size) {
      const auto resultOffset = offset;
      offset += size;
      return {buff, resultOffset};
    }
  }

  // If no transfer buffers has enough space then create a new one.
  const auto newBuffSize = size > g_minTransferBufferSize ? size : g_minTransferBufferSize;
  m_transferBuffers.emplace_back(
      Buffer{m_device, newBuffSize, MemoryLocation::Host, BufferUsage::Transfer},
      static_cast<uint32_t>(size));

  LOG_D(m_logger, "Vulkan transfer buffer created", {"size", log::MemSize{newBuffSize}});

  return {m_transferBuffers.back().first, 0U};
}

} // namespace tria::gfx::internal

#include "memory_pool.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

constexpr auto g_minChunkSize                   = 64U * 1024U * 1024U;
constexpr auto g_chunkInitialFreeBlocksCapacity = 128U;

[[nodiscard]] auto getVkMemoryProperties(MemoryLocation loc) noexcept -> VkMemoryPropertyFlagBits {
  switch (loc) {
  case MemoryLocation::Device:
    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  case MemoryLocation::Host:
  default:
    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }
}

[[nodiscard]] auto allocVkMemory(VkDevice vkDevice, uint32_t size, uint32_t memoryType)
    -> VkDeviceMemory {
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = size;
  allocInfo.memoryTypeIndex      = memoryType;

  VkDeviceMemory result;
  checkVkResult(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &result));
  return result;
};

[[nodiscard]] auto mapVkMemory(VkDevice vkDevice, VkDeviceMemory vkMemory) -> char* {
  void* map;
  checkVkResult(vkMapMemory(vkDevice, vkMemory, 0, VK_WHOLE_SIZE, 0, &map));
  return static_cast<char*>(map);
}

[[nodiscard]] constexpr auto getName(MemoryLocation loc) noexcept -> std::string_view {
  switch (loc) {
  case MemoryLocation::Host:
    return "host";
  case MemoryLocation::Device:
    return "device";
  }
  return "unknown";
}

/* Check if the given memory blocks overlap.
 */
[[maybe_unused]] [[nodiscard]] auto doesOverlap(const MemoryBlock& a, const MemoryBlock& b) {
  return a.getEndOffset() > b.getOffset() && a.getOffset() < b.getEndOffset();
}

} // namespace

MemoryBlock::~MemoryBlock() {
  if (m_chunk) {
    m_chunk->free(this);
  }
}

auto MemoryBlock::bindToBuffer(VkBuffer buffer) -> void {
  assert(m_chunk);
  checkVkResult(
      vkBindBufferMemory(m_chunk->getVkDevice(), buffer, m_chunk->getVkMemory(), m_offset));
}

auto MemoryBlock::getMappedPtr() const noexcept -> char* {
  assert(m_chunk);
  auto* basePtr = m_chunk->getMappedPtr();
  return basePtr ? basePtr + m_offset : nullptr;
}

auto MemoryBlock::flush() -> void {
  assert(m_chunk);
  m_chunk->flush(m_offset, m_size);
}

MemoryChunk::MemoryChunk(
    log::Logger* logger,
    VkDevice vkDevice,
    MemoryLocation loc,
    uint32_t memoryType,
    uint32_t size,
    uint32_t flushAlignment) :
    m_logger{logger},
    m_vkDevice{vkDevice},
    m_loc{loc},
    m_memType{memoryType},
    m_size{size},
    m_flushAlignment{flushAlignment} {

  // Allocate backing memory from Vulkan.
  m_vkMemory = allocVkMemory(vkDevice, size, m_memType);

  // Start with a single free block spanning the whole size.
  m_freeBlocks.reserve(g_chunkInitialFreeBlocksCapacity);
  m_freeBlocks.push_back(MemoryBlock{nullptr, 0U, size});

  assert(getFreeSize() == m_size);
  assert(getOccupiedSize() == 0U);

  // For host memory we map the range so we can write into it.
  m_map = loc == MemoryLocation::Host ? mapVkMemory(m_vkDevice, m_vkMemory) : nullptr;

  LOG_I(
      m_logger,
      "Vulkan memory chunk allocated",
      {"location", getName(m_loc)},
      {"type", m_memType},
      {"size", log::MemSize{size}},
      {"flushAlignment", log::MemSize{m_flushAlignment}});
}

MemoryChunk::~MemoryChunk() {
  // Check that all blocks are properly freed.
  assert(getFreeSize() == m_size);
  assert(getOccupiedSize() == 0);

  if (m_loc == MemoryLocation::Host) {
    vkUnmapMemory(m_vkDevice, m_vkMemory);
  }
  vkFreeMemory(m_vkDevice, m_vkMemory, nullptr);

  LOG_I(m_logger, "Vulkan memory chunk freed", {"location", getName(m_loc)}, {"type", m_memType});
}

auto MemoryChunk::allocate(uint32_t alignment, uint32_t size) noexcept
    -> std::optional<MemoryBlock> {

#if !defined(NDEBUG)
  const auto dbgFreeSize = getFreeSize();
#endif

  // Find a free block that fits the requested size (while respecting the alignment).
  for (auto i = 0U; i != m_freeBlocks.size(); ++i) {
    const auto freeBlockOffset = m_freeBlocks[i].getOffset();
    const auto freeBlockSize   = m_freeBlocks[i].getSize();

    const auto padding    = padToAlignment(freeBlockOffset, alignment);
    const auto paddedSize = size + padding;
    const auto remainingSize =
        static_cast<int32_t>(freeBlockSize) - static_cast<int32_t>(paddedSize);

    if (remainingSize < 0) {
      // Doesn't fit in this block.
      continue;
    }

    if (padding > 0) {
      // We are left with the space that was taken up by the padding, add it as a new block.
      m_freeBlocks.push_back(MemoryBlock{nullptr, freeBlockOffset, padding});
    }

    // Either shrink the block to 'remove' the space we need, or remove the block if we need all the
    // space.
    if (remainingSize > 0) {
      m_freeBlocks[i].m_offset += paddedSize;
      m_freeBlocks[i].m_size = remainingSize;
    } else {
      m_freeBlocks.erase(m_freeBlocks.begin() + i);
    }

    assert(dbgFreeSize - getFreeSize() == size);
    return MemoryBlock{this, freeBlockOffset + padding, size};
  }

  // No block can fit the requested size.
  return std::nullopt;
}

auto MemoryChunk::free(MemoryBlock* block) noexcept -> void {
  assert(block);
  assert(block->m_chunk == this);

#if !defined(NDEBUG)
  // Sanity check that this block was not freed before.
  for (const auto& freeBlock : m_freeBlocks) {
    assert(!doesOverlap(*block, freeBlock));
  }
  const auto dbgFreeSize = getFreeSize();
#endif

  // Unset the chunk pointer to indicate that the block has already been returned.
  block->m_chunk = nullptr;

  // Check if there already is a free block before or after this one, if so then 'grow' that one.
  // TODO(bastian): Instead of a linear scan we can consider using a ordered_map with offset as the
  // key, that would allow us to find the neighbors much faster.
  for (auto& freeBlock : m_freeBlocks) {

    // Check if this freeBlock is right before the given block.
    if (freeBlock.getEndOffset() == block->getOffset()) {
      freeBlock.m_size += block->getSize();
      assert(getFreeSize() - dbgFreeSize == block->getSize());
      return;
    }

    // Check if this freeBlock is right after the given block.
    if (freeBlock.getOffset() == block->getEndOffset()) {
      freeBlock.m_offset -= block->getSize();
      freeBlock.m_size += block->getSize();
      assert(getFreeSize() - dbgFreeSize == block->getSize());
      return;
    }
  }

  // No block to join, add as a new block.
  m_freeBlocks.push_back(MemoryBlock{nullptr, block->m_offset, block->m_size});
  assert(getFreeSize() - dbgFreeSize == block->getSize());
}

auto MemoryChunk::flush(uint32_t offset, uint32_t size) -> void {
  assert(m_map); // Only mapped memory can be flushed.

  // Align the offset to be a multiple of 'flushAlignment'.
  auto alignedOffset = offset / m_flushAlignment * m_flushAlignment;
  assert(offset >= alignedOffset && offset - alignedOffset < m_flushAlignment);

  // Pad the size to be aligned (or until the end of the chunk).
  auto paddedSize = size + padToAlignment(size, m_flushAlignment);
  if (offset + paddedSize > m_size) {
    paddedSize = m_size - offset;
  }

  VkMappedMemoryRange mappedMemoryRange = {};
  mappedMemoryRange.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedMemoryRange.memory              = m_vkMemory;
  mappedMemoryRange.offset              = alignedOffset;
  mappedMemoryRange.size                = paddedSize;
  checkVkResult(vkFlushMappedMemoryRanges(m_vkDevice, 1, &mappedMemoryRange));
}

auto MemoryChunk::getFreeSize() const noexcept -> uint32_t {
  uint32_t result = 0U;
  for (const auto& chunk : m_freeBlocks) {
    result += chunk.m_size;
  }
  return result;
}

auto MemoryPool::allocate(MemoryLocation location, VkMemoryRequirements requirements)
    -> MemoryBlock {
  return allocate(
      location,
      static_cast<uint32_t>(requirements.alignment),
      static_cast<uint32_t>(requirements.size),
      requirements.memoryTypeBits);
}

auto MemoryPool::allocate(
    MemoryLocation location, uint32_t alignment, uint32_t size, uint32_t supportedMemoryTypes)
    -> MemoryBlock {

  // Attempt to allocate from an existing chunk.
  for (auto& chunk : m_chunks) {
    const auto isSupported = (supportedMemoryTypes & (1U << chunk.getMemType())) != 0;
    if (chunk.getLocation() == location && isSupported) {
      auto allocation = chunk.allocate(alignment, size);
      if (allocation) {
        return std::move(*allocation);
      }
    }
  }

  // If no existing chunk has space then create a new chunk.
  const auto newChunkSize    = size > g_minChunkSize ? size : g_minChunkSize;
  const auto newChunkMemType = getMemoryType(getVkMemoryProperties(location), supportedMemoryTypes);
  m_chunks.emplace_front(
      m_logger,
      m_vkDevice,
      location,
      newChunkMemType,
      newChunkSize,
      static_cast<uint32_t>(m_deviceLimits.nonCoherentAtomSize));

  // Allocate from the new chunk.
  return *m_chunks.front().allocate(alignment, size);
}

auto MemoryPool::getMemoryType(VkMemoryPropertyFlags properties, uint32_t allowedMemTypes)
    -> uint32_t {
  // Find a memory-type that is allowed from the 'allowedMemTypes' bit-mask and that satisfies the
  // requested properties.
  for (uint32_t i = 0; i < m_deviceMemProperties.memoryTypeCount; i++) {
    const auto isAllowed = allowedMemTypes & (1U << i);
    const auto hasProperties =
        (m_deviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties;
    if (isAllowed && hasProperties) {
      return i;
    }
  }
  throw err::GfxErr{"Device has no memory type that satisfies required properties"};
}

} // namespace tria::gfx::internal

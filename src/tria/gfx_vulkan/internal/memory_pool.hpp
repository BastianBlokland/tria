#pragma once
#include "tria/log/api.hpp"
#include <cstdint>
#include <forward_list>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class MemoryChunk;

enum class MemoryLocation {
  Host,   // Can be written to from the cpu side.
  Device, // Memory on the gpu itself, memory need to be explicitly transferred.
};

enum class MemoryAccessType {
  Linear,    // Normal memory (for example buffers).
  NonLinear, // Images that use a tiling mode different then 'VK_IMAGE_TILING_LINEAR'.
};

/* Continuous block of allocated memory.
 * Is automatically freed in the destructor.
 */
class MemoryBlock final {
  friend MemoryChunk;

public:
  MemoryBlock() : m_chunk{nullptr}, m_offset{0U}, m_size{0U} {}
  MemoryBlock(const MemoryBlock& rhs) = delete;
  MemoryBlock(MemoryBlock&& rhs) noexcept {
    m_chunk     = rhs.m_chunk;
    m_offset    = rhs.m_offset;
    m_size      = rhs.m_size;
    rhs.m_chunk = nullptr; // Indicate that the 'moved-from' block does not belong to a chunk.
  };
  ~MemoryBlock();

  auto operator=(const MemoryBlock& rhs) -> MemoryBlock& = delete;
  auto operator=(MemoryBlock&& rhs) noexcept -> MemoryBlock& {
    m_chunk     = rhs.m_chunk;
    m_offset    = rhs.m_offset;
    m_size      = rhs.m_size;
    rhs.m_chunk = nullptr; // Indicate that the 'moved-from' block does not belong to a chunk.
    return *this;
  }

  /* Offset into the memory chunk that this block starts at.
   */
  [[nodiscard]] auto getOffset() const noexcept -> uint32_t { return m_offset; }

  /* Size of the memory block.
   */
  [[nodiscard]] auto getSize() const noexcept -> uint32_t { return m_size; }

  /* Offset into the memory chunk that this block ends at.
   */
  [[nodiscard]] auto getEndOffset() const noexcept -> uint32_t { return m_offset + m_size; }

  /* Use this block as backing memory for the given buffer.
   */
  auto bindToBuffer(VkBuffer buffer) -> void;

  /* Use this block as backing memory for the given image.
   */
  auto bindToImage(VkImage image) -> void;

  /* Get a mapped pointer to write into.
   * Note: Only valid for 'Host' allocations.
   */
  [[nodiscard]] auto getMappedPtr() const noexcept -> char*;

  /* Flush the mapped memory. Call this after modifying the mapped memory.
   * Note: Only valid for 'Host' allocations.
   */
  auto flush() -> void;

private:
  MemoryChunk* m_chunk;
  uint32_t m_offset;
  uint32_t m_size;

  MemoryBlock(MemoryChunk* chunk, uint32_t offset, uint32_t size) :
      m_chunk{chunk}, m_offset{offset}, m_size{size} {};
};

/* Single continuous chunk of memory that blocks can be allocated from.
 * Note: Api is NOT thread-safe.
 */
class MemoryChunk final {
public:
  MemoryChunk() = delete;
  MemoryChunk(
      log::Logger* logger,
      VkDevice vkDevice,
      MemoryLocation loc,
      MemoryAccessType accessType,
      uint32_t memoryType,
      uint32_t size,
      uint32_t flushAlignment);
  MemoryChunk(const MemoryChunk& rhs) = delete;
  MemoryChunk(MemoryChunk&& rhs)      = delete;
  ~MemoryChunk();

  auto operator=(const MemoryChunk& rhs) -> MemoryChunk& = delete;
  auto operator=(MemoryChunk&& rhs) -> MemoryChunk& = delete;

  [[nodiscard]] auto getLocation() const noexcept { return m_loc; }
  [[nodiscard]] auto getMemType() const noexcept { return m_memType; }
  [[nodiscard]] auto getMemAccessType() const noexcept { return m_accessType; }
  [[nodiscard]] auto getVkDevice() const noexcept { return m_vkDevice; }
  [[nodiscard]] auto getVkMemory() const noexcept { return m_vkMemory; }
  [[nodiscard]] auto getMappedPtr() const noexcept { return m_map; }

  /* Allocate a block of memory.
   */
  [[nodiscard]] auto allocate(uint32_t alignment, uint32_t size) noexcept
      -> std::optional<MemoryBlock>;

  /* Free a previously allocated block.
   * Note: block needs to be allocated from this chunk and not freed before.
   */
  auto free(MemoryBlock* block) noexcept -> void;

  /* Flush the given memory range.
   * Note: Only valid for 'Host' allocations.
   */
  auto flush(uint32_t offset, uint32_t size) -> void;

private:
  log::Logger* m_logger;
  VkDevice m_vkDevice;
  MemoryLocation m_loc;
  MemoryAccessType m_accessType;
  uint32_t m_memType;
  uint32_t m_size;
  uint32_t m_flushAlignment; // Alignment that must be respected when flushing mapped memory.
  VkDeviceMemory m_vkMemory;
  std::vector<MemoryBlock> m_freeBlocks;
  char* m_map;

  [[nodiscard]] auto getFreeSize() const noexcept -> uint32_t;
  [[nodiscard]] auto getOccupiedSize() const noexcept { return m_size - getFreeSize(); }
};

/* Device memory allocator.
 * Note: Api is NOT thread-safe.
 */
class MemoryPool final {
public:
  MemoryPool(
      log::Logger* logger,
      VkDevice vkDevice,
      const VkPhysicalDeviceMemoryProperties& deviceMemProperties,
      const VkPhysicalDeviceLimits& deviceLimits) :
      m_logger{logger},
      m_vkDevice{vkDevice},
      m_deviceMemProperties{deviceMemProperties},
      m_deviceLimits{deviceLimits} {}
  MemoryPool(const MemoryPool& rhs) = delete;
  MemoryPool(MemoryPool&& rhs)      = delete;
  ~MemoryPool()                     = default;

  auto operator=(const MemoryPool& rhs) -> MemoryPool& = delete;
  auto operator=(MemoryPool&& rhs) -> MemoryPool& = delete;

  /* Allocate a block of memory that satisfies the given requirements.
   */
  [[nodiscard]] auto
  allocate(MemoryLocation location, MemoryAccessType accessType, VkMemoryRequirements requirements)
      -> MemoryBlock;

  /* Allocate a block of memory.
   * 'allowedMemTypes' is a bitmask that can be used to specify which memoryTypes may be used for
   * this allocation.
   */
  [[nodiscard]] auto allocate(
      MemoryLocation location,
      MemoryAccessType accessType,
      uint32_t alignment,
      uint32_t size,
      uint32_t allowedMemTypes = ~0U) -> MemoryBlock;

private:
  log::Logger* m_logger;
  VkDevice m_vkDevice;
  const VkPhysicalDeviceMemoryProperties& m_deviceMemProperties;
  const VkPhysicalDeviceLimits& m_deviceLimits;
  std::forward_list<MemoryChunk> m_chunks;

  [[nodiscard]] auto getMemoryType(VkMemoryPropertyFlags properties, uint32_t allowedMemTypes)
      -> uint32_t;
};

using MemoryPoolUnique = std::unique_ptr<MemoryPool>;

} // namespace tria::gfx::internal

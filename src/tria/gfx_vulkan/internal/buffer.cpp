#include "buffer.hpp"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getVkBufferUsageFlags(BufferUsage usage) -> VkBufferUsageFlags {
  switch (usage) {
  case BufferUsage::VertexData:
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  case BufferUsage::Transfer:
    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }
  throw err::DriverErr{"Unexpected buffer-usage"};
}

[[nodiscard]] auto getVkMemoryRequirements(VkDevice vkDevice, VkBuffer vkBuffer)
    -> VkMemoryRequirements {
  VkMemoryRequirements result;
  vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &result);
  return result;
}

[[nodiscard]] auto createVkBuffer(VkDevice vkDevice, uint32_t size, VkBufferUsageFlags usageFlags)
    -> VkBuffer {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size        = size;
  bufferInfo.usage       = usageFlags;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer result;
  checkVkResult(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &result));
  return result;
}

} // namespace

Buffer::Buffer(Device* device, size_t size, MemoryLocation location, BufferUsage usage) :
    m_device{device} {
  assert(m_device);

  // Create a buffer.
  auto usageFlags = getVkBufferUsageFlags(usage);
  m_vkBuffer      = createVkBuffer(device->getVkDevice(), static_cast<uint32_t>(size), usageFlags);

  // Allocate memory for it.
  auto memoryRequirements = getVkMemoryRequirements(device->getVkDevice(), m_vkBuffer);
  m_memory                = device->getMemory().allocate(location, memoryRequirements);

  // Bind the memory to the buffer.
  m_memory.bindToBuffer(m_vkBuffer);
}

Buffer::~Buffer() {
  if (m_vkBuffer) {
    vkDestroyBuffer(m_device->getVkDevice(), m_vkBuffer, nullptr);
  }
  // Note: Memory is reclaimed by the destructor of MemoryBlock.
}

auto Buffer::upload(const void* data, size_t size) -> void {
  if (!m_vkBuffer) {
    throw err::DriverErr{"Invalid buffer"};
  }
  if (size > m_memory.getSize()) {
    throw err::DriverErr{"Buffer too small"};
  }
  auto* mappedPtr = m_memory.getMappedPtr();
  if (!mappedPtr) {
    throw err::DriverErr{"Unable to map buffer memory"};
  }
  std::memcpy(mappedPtr, data, size);
  m_memory.flush();
}

} // namespace tria::gfx::internal

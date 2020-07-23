#pragma once
#include "device.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

enum class BufferUsage {
  VertexData,
  Transfer,
};

/* Data buffer.
 * Either residing on the host (cpu) side or on the device (gpu) side.
 */
class Buffer final {
public:
  Buffer() = default;
  Buffer(Device* device, size_t size, MemoryLocation loc, BufferUsage usage);
  Buffer(const Buffer& rhs) = delete;
  Buffer(Buffer&& rhs)      = delete;
  ~Buffer();

  auto operator=(const Buffer& rhs) -> Buffer& = delete;

  auto operator=(Buffer&& rhs) noexcept -> Buffer& {
    m_device       = rhs.m_device;
    m_vkBuffer     = rhs.m_vkBuffer;
    m_memory       = std::move(rhs.m_memory);
    rhs.m_vkBuffer = nullptr; // Indicate to the moved-from buffer that we will destroy it.
    return *this;
  }

  [[nodiscard]] auto getVkBuffer() const noexcept { return m_vkBuffer; }

  /* Upload data to the buffer.
   * Note: Only valid for buffers on the host (cpu) side. For gpu buffers an explicit transfer
   * operation has to be used.
   */
  auto upload(const void* data, size_t size) -> void;

private:
  const Device* m_device;
  VkBuffer m_vkBuffer;
  MemoryBlock m_memory;
};

} // namespace tria::gfx::internal

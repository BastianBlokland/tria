#pragma once
#include "buffer.hpp"
#include "device.hpp"
#include "image.hpp"
#include "tria/log/api.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Memory transferer.
 * Responsible for transfering memory from the cpu side to the gpu side.
 * Transfers can be 'queued' to the transferer and then batch recorded to a command-buffer.
 */
class Transferer final {
public:
  Transferer(log::Logger* logger, Device* device) : m_logger{logger}, m_device{device} {}
  ~Transferer() = default;

  /* Reset the transferer.
   * This clears any queued work and frees up the used transfer buffers.
   * Note: Only call this after the device has finished executing the command-buffer containing the
   * transfer operations.
   * Usually called at the beginning of a frame.
   */
  auto reset() noexcept -> void;

  /* Queue a transfer of the specified memory to the destination buffer.
   */
  auto queueTransfer(const void* data, const Buffer& dst, size_t dstOffset, size_t size) -> void;

  /* Queue a transfer of the specified memory to the destination image.
   * Note: Care must be taken that there is enough data available to fill the image.
   */
  auto queueTransfer(const void* data, const Image& dst) -> void;

  /* Record transfer commands for the queued work.
   * Note: clears queued transfer items, so recording can be done in batches. However the required
   * transfer buffers are only cleared when calling 'reset'.
   */
  auto record(VkCommandBuffer buffer) noexcept -> void;

private:
  struct BufferWork final {
    std::pair<const Buffer&, uint32_t> src;
    const Buffer& dst;
    size_t dstOffset;
    size_t size;

    BufferWork(
        std::pair<const Buffer&, uint32_t> src, const Buffer& dst, size_t dstOffset, size_t size) :
        src{src}, dst{dst}, dstOffset{dstOffset}, size{size} {}
  };

  struct ImageWork final {
    std::pair<const Buffer&, uint32_t> src;
    const Image& dst;

    ImageWork(std::pair<const Buffer&, uint32_t> src, const Image& dst) : src{src}, dst{dst} {}
  };

  log::Logger* m_logger;
  Device* m_device;
  std::vector<std::pair<Buffer, uint32_t>> m_transferBuffers;
  std::vector<BufferWork> m_bufferWork;
  std::vector<ImageWork> m_imageWork;

  /* Get a buffer (and an offset into that buffer) to use as a transfer buffer.
   */
  [[nodiscard]] auto getTransferSpace(size_t size, size_t alignment)
      -> std::pair<Buffer&, uint32_t>;
};

using TransfererUnique = std::unique_ptr<Transferer>;

} // namespace tria::gfx::internal

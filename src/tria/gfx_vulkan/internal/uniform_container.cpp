#include "uniform_container.hpp"
#include "debug_utils.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

/* Maximum amount of data that we can bind to a single uniform, note: might be lower if the
 * 'maxUniformBufferRange' hardware limit is lower then this.
 *
 * We can alias memory bindings (while respecting 'minUniformBufferOffsetAlignment') so not every
 * element will consume this much, however we need at least this much space at the end of the
 * buffer. Reason is Vulkan cannot know that we will not read out of bounds so memory might be
 * wasted at the end of buffers.
 */
constexpr auto g_desiredMaxDataSize = 64U * 1024U;

/* Size of the backing buffers to allocate.
 */
constexpr auto g_uniformBufferSize = 32U * 1024U * 1024U;

} // namespace

UniformContainer::UniformContainer(log::Logger* logger, Device* device) :
    m_logger{logger}, m_device{device} {
  assert(m_device);

  m_descInfo     = DescriptorInfo{0U, 1U, 0U};
  m_minAlignment = m_device->getLimits().minUniformBufferOffsetAlignment;
  m_maxDataSize  = std::min(m_device->getLimits().maxUniformBufferRange, g_desiredMaxDataSize);
}

auto UniformContainer::reset() noexcept -> void {
  // Reset the taken offset on all sets.
  for (auto& set : m_sets) {
    set.offset = 0U;
  }
}

auto UniformContainer::upload(const void* data, size_t size)
    -> std::pair<VkDescriptorSet, uint32_t> {
  assert(data);
  assert(size > 0U);

  const auto uintSize   = static_cast<uint32_t>(size);
  const auto padding    = padToAlignment(uintSize, m_minAlignment);
  const auto paddedSize = uintSize + padding;
  if (paddedSize > m_maxDataSize) {
    throw err::GfxErr{"Uniform data size exceeds maximum"};
  }

  // Find space in an existing set.
  for (auto& set : m_sets) {
    // Check if this set still has enough space left.
    // Note: Even though there is only 'size' amount of space requested we still need to ensure that
    // at least up to 'm_maxDataSize' is available, reason is we told Vulkan to bind up to that
    // much data and it cannot know that we will not be using all of it.
    if (set.buffer.getSize() - set.offset >= m_maxDataSize) {

      const auto resultOffset = set.offset;
      set.buffer.upload(data, size, resultOffset);

      set.offset += paddedSize;
      return {set.descSet.getVkDescSet(), resultOffset};
    }
  }

  // If no set has enough space then create a new one.
  auto descSet = m_device->getDescManager().allocate(m_descInfo);
  auto buffer =
      Buffer{m_device, g_uniformBufferSize, MemoryLocation::Host, BufferUsage::HostUniformData};

  DBG_BUFFER_NAME(m_device, buffer.getVkBuffer(), "uniform_container");

  descSet.attachUniformBufferDynamic(0U, buffer, m_maxDataSize);
  m_sets.emplace_back(std::move(descSet), std::move(buffer), paddedSize);

  LOG_D(
      m_logger,
      "Vulkan dynamic uniform buffer created",
      {"size", log::MemSize{g_uniformBufferSize}},
      {"maxDataSize", log::MemSize{m_maxDataSize}},
      {"minAlignment", log::MemSize{m_minAlignment}});

  m_sets.back().buffer.upload(data, size);
  return {m_sets.back().descSet.getVkDescSet(), 0U};
}

} // namespace tria::gfx::internal

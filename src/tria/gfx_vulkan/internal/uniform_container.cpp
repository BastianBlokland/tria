#include "uniform_container.hpp"
#include "debug_utils.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

// Arbitrary limit of how much data is allowed to be send to a descriptor.
// TODO(bastian): needs profiling to see if lowering this limit improves performance, if so it might
// make sense to use multiple descriptors for different sizes.
constexpr auto g_maxUniformDataSize = 2U * 1024U;

constexpr auto g_uniformBufferSize = 1024U * 1024U;

} // namespace

UniformContainer::UniformContainer(log::Logger* logger, Device* device) :
    m_logger{logger}, m_device{device} {
  assert(m_device);

  m_descInfo     = DescriptorInfo{0U, 1U};
  m_minAlignment = m_device->getLimits().minUniformBufferOffsetAlignment;
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
  if (paddedSize > g_maxUniformDataSize) {
    throw err::GfxErr{"Uniform data size exceeds maximum"};
  }

  // Find space in an existing set.
  for (auto& set : m_sets) {
    // Check if this set still has enough space left.
    if (set.buffer.getSize() - set.offset >= g_maxUniformDataSize) {

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

  descSet.attachUniformBufferDynamic(0U, buffer, g_maxUniformDataSize);
  m_sets.emplace_back(std::move(descSet), std::move(buffer), paddedSize);

  LOG_D(
      m_logger,
      "Vulkan dynamic uniform buffer created",
      {"size", log::MemSize{g_uniformBufferSize}},
      {"maxUniformDataSize", log::MemSize{g_maxUniformDataSize}},
      {"minAlignment", log::MemSize{m_minAlignment}});

  m_sets.back().buffer.upload(data, size);
  return {m_sets.back().descSet.getVkDescSet(), 0U};
}

} // namespace tria::gfx::internal

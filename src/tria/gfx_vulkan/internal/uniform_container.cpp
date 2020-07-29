#include "uniform_container.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

// Vulkan spec specifies at least 8 dynamic uniform buffers should be available.
constexpr auto g_maxUniformBuffers = 4U;

// Arbitrary limit of how much data is allowed to be send to a descriptor.
// TODO(bastian): needs profiling to see if lowering this limit improves performance, if so it might
// make sense to use multiple descriptors for different sizes.
constexpr auto g_maxUniformDataSize = 2U * 1024U;

constexpr auto g_uniformBufferSize = 1024U * 1024U;

[[nodiscard]] auto createVkDescriptorSetLayout(const Device* device) -> VkDescriptorSetLayout {
  VkDescriptorSetLayoutBinding uniformBinding = {};
  uniformBinding.binding                      = 0U;
  uniformBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  uniformBinding.descriptorCount              = 1U;
  uniformBinding.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount                    = 1U;
  layoutInfo.pBindings                       = &uniformBinding;

  VkDescriptorSetLayout result;
  checkVkResult(vkCreateDescriptorSetLayout(device->getVkDevice(), &layoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkDescriptorPool(const Device* device) -> VkDescriptorPool {
  VkDescriptorPoolSize poolSize = {};
  poolSize.type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  poolSize.descriptorCount      = g_maxUniformBuffers;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount              = 1U;
  poolInfo.pPoolSizes                 = &poolSize;
  poolInfo.maxSets                    = g_maxUniformBuffers;

  VkDescriptorPool result;
  checkVkResult(vkCreateDescriptorPool(device->getVkDevice(), &poolInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto allocVkDescriptorSet(
    const Device* device, const VkDescriptorPool pool, const VkDescriptorSetLayout layout)
    -> VkDescriptorSet {

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool              = pool;
  allocInfo.descriptorSetCount          = 1U;
  allocInfo.pSetLayouts                 = &layout;

  VkDescriptorSet result;
  checkVkResult(vkAllocateDescriptorSets(device->getVkDevice(), &allocInfo, &result));
  return result;
}

auto configureVkDescriptorSet(const Device* device, VkDescriptorSet descSet, const Buffer& buffer)
    -> void {
  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer                 = buffer.getVkBuffer();
  bufferInfo.offset                 = 0U;
  bufferInfo.range                  = g_maxUniformDataSize;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = descSet;
  descriptorWrite.dstBinding           = 0U;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pBufferInfo          = &bufferInfo;

  vkUpdateDescriptorSets(device->getVkDevice(), 1U, &descriptorWrite, 0U, nullptr);
}

} // namespace

UniformContainer::UniformContainer(log::Logger* logger, Device* device) :
    m_logger{logger}, m_device{device} {
  assert(m_device);

  m_minAlignment = m_device->getLimits().minUniformBufferOffsetAlignment;
  m_vkDescLayout = createVkDescriptorSetLayout(m_device);
  m_vkDescPool   = createVkDescriptorPool(m_device);
}

UniformContainer::~UniformContainer() {
  vkDestroyDescriptorSetLayout(m_device->getVkDevice(), m_vkDescLayout, nullptr);
  vkDestroyDescriptorPool(m_device->getVkDevice(), m_vkDescPool, nullptr);
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
      return {set.descSet, resultOffset};
    }
  }

  // If no set has enough space then create a new one.

  if (m_sets.size() == g_maxUniformBuffers) {
    throw err::GfxErr{"Unable to allocate new uniform buffer: maximum reached"};
  }

  const auto descSet = allocVkDescriptorSet(m_device, m_vkDescPool, m_vkDescLayout);
  auto buffer =
      Buffer{m_device, g_uniformBufferSize, MemoryLocation::Host, BufferUsage::HostUniformData};
  configureVkDescriptorSet(m_device, descSet, buffer);
  m_sets.emplace_back(descSet, std::move(buffer), paddedSize);

  LOG_D(
      m_logger,
      "Vulkan dynamic uniform buffer created",
      {"size", log::MemSize{g_uniformBufferSize}},
      {"maxUniformDataSize", log::MemSize{g_maxUniformDataSize}},
      {"minAlignment", log::MemSize{m_minAlignment}});

  m_sets.back().buffer.upload(data, size);
  return {m_sets.back().descSet, 0U};
}

} // namespace tria::gfx::internal

#include "descriptor_manager.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "image_sampler.hpp"
#include "tria/math/utils.hpp"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkDescriptorSetLayout(VkDevice vkDevice, const DescriptorInfo& info)
    -> VkDescriptorSetLayout {

  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(info.getBindingCount());

  auto binding = 0U;

  // TODO(bastian): The stage-flags should probably be configable somehow.

  // Images.
  for (auto i = 0U; i != info.getImageCount(); ++i, ++binding) {
    VkDescriptorSetLayoutBinding imgBinding = {};
    imgBinding.binding                      = binding;
    imgBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imgBinding.descriptorCount              = 1U;
    imgBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(imgBinding);
  }

  // Uniform buffers.
  for (auto i = 0U; i != info.getUniformBufferCount(); ++i, ++binding) {
    VkDescriptorSetLayoutBinding buffBinding = {};
    buffBinding.binding                      = binding;
    buffBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    buffBinding.descriptorCount              = 1U;
    buffBinding.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;
    bindings.push_back(buffBinding);
  }

  // Dynamic uniform buffers.
  for (auto i = 0U; i != info.getUniformBufferDynamicCount(); ++i, ++binding) {
    VkDescriptorSetLayoutBinding buffBindingDyn = {};
    buffBindingDyn.binding                      = binding;
    buffBindingDyn.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    buffBindingDyn.descriptorCount              = 1U;
    buffBindingDyn.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;
    bindings.push_back(buffBindingDyn);
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount                    = bindings.size();
  layoutInfo.pBindings                       = bindings.data();

  VkDescriptorSetLayout result;
  checkVkResult(vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkDescriptorPool(VkDevice vkDevice, const DescriptorInfo& info)
    -> VkDescriptorPool {
  constexpr auto maxDescPoolSizes = 3U;

  std::array<VkDescriptorPoolSize, maxDescPoolSizes> sizes;
  auto sizesCount = 0U;

  // Images.
  if (info.getImageCount() > 0) {
    sizes[sizesCount].type              = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sizes[sizesCount++].descriptorCount = info.getImageCount() * g_descriptorSetsPerGroup;
  }

  // Uniform buffers.
  if (info.getUniformBufferCount() > 0) {
    sizes[sizesCount].type              = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[sizesCount++].descriptorCount = info.getUniformBufferCount() * g_descriptorSetsPerGroup;
  }

  // Dynamic uniform buffers.
  if (info.getUniformBufferDynamicCount() > 0) {
    sizes[sizesCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    sizes[sizesCount++].descriptorCount =
        info.getUniformBufferDynamicCount() * g_descriptorSetsPerGroup;
  }

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount              = sizesCount;
  poolInfo.pPoolSizes                 = sizes.data();
  poolInfo.maxSets                    = g_descriptorSetsPerGroup;

  VkDescriptorPool result;
  checkVkResult(vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &result));
  return result;
}

template <unsigned int Count>
auto allocVkDescriptorSets(
    VkDevice vkDevice,
    const VkDescriptorPool pool,
    const VkDescriptorSetLayout layout,
    std::array<VkDescriptorSet, Count>& output) -> void {

  // Use the same layout for all allocations.
  std::array<VkDescriptorSetLayout, Count> layouts;
  for (auto i = 0U; i != Count; ++i) {
    layouts[i] = layout;
  }

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool              = pool;
  allocInfo.descriptorSetCount          = Count;
  allocInfo.pSetLayouts                 = layouts.data();

  checkVkResult(vkAllocateDescriptorSets(vkDevice, &allocInfo, output.data()));
}

} // namespace

DescriptorSet::~DescriptorSet() noexcept {
  if (m_group) {
    m_group->free(this);
    m_group = nullptr;
  }
}

auto DescriptorSet::getInfo() const noexcept -> const DescriptorInfo& { return m_group->getInfo(); }

auto DescriptorSet::getVkLayout() const noexcept -> VkDescriptorSetLayout {
  return m_group ? m_group->getVkLayout() : nullptr;
}

auto DescriptorSet::getVkDescSet() const noexcept -> VkDescriptorSet {
  return m_group ? m_group->getVkDescSet(m_id) : nullptr;
}

auto DescriptorSet::bindImage(uint32_t binding, const Image& img, const ImageSampler& sampler)
    -> void {
  m_group->bindImage(this, binding, img, sampler);
}

auto DescriptorSet::bindUniformBuffer(uint32_t binding, const Buffer& buffer) -> void {
  m_group->bindUniformBuffer(this, binding, buffer);
}

auto DescriptorSet::bindUniformBufferDynamic(
    uint32_t binding, const Buffer& buffer, uint32_t maxSize) -> void {
  m_group->bindUniformBufferDynamic(this, binding, buffer, maxSize);
}

DescriptorGroup::DescriptorGroup(log::Logger* logger, VkDevice vkDevice, DescriptorInfo info) :
    m_logger{logger}, m_vkDevice{vkDevice}, m_info{info} {

  m_vkPool   = createVkDescriptorPool(vkDevice, m_info);
  m_vkLayout = createVkDescriptorSetLayout(vkDevice, m_info);

  // Preallocate all the descriptor sets.
  allocVkDescriptorSets<g_descriptorSetsPerGroup>(m_vkDevice, m_vkPool, m_vkLayout, m_sets);

  // Mark the allocated sets as 'free'.
  m_free = 0U;
  for (auto i = 0U; i != g_descriptorSetsPerGroup; ++i) {
    m_free |= 1 << i;
  }

  LOG_I(
      m_logger,
      "Vulkan descriptor group allocated",
      {"setCount", g_descriptorSetsPerGroup},
      {"setImgCount", info.getImageCount()},
      {"setUniformBuffCount", info.getUniformBufferCount()},
      {"setUniformBuffDynamicCount", info.getUniformBufferDynamicCount()});
}

DescriptorGroup::~DescriptorGroup() {
#if !defined(NDEBUG)
  // Check that all sets are properly freed.
  for (auto i = 0U; i != g_descriptorSetsPerGroup; ++i) {
    assert((m_free & (1 << i)) != 0);
  }
#endif

  vkDestroyDescriptorPool(m_vkDevice, m_vkPool, nullptr);
  vkDestroyDescriptorSetLayout(m_vkDevice, m_vkLayout, nullptr);

  LOG_I(m_logger, "Vulkan descriptor group freed");
}

auto DescriptorGroup::allocate() noexcept -> std::optional<DescriptorSet> {
  if (m_free == 0U) {
    return std::nullopt;
  }
  auto tz = math::countTrailingZeroes(m_free); // Find the index of the first free set.
  m_free &= ~(1U << tz);                       // Un-mark the set as free.
  return DescriptorSet{this, static_cast<int32_t>(tz)};
}

auto DescriptorGroup::bindImage(
    DescriptorSet* set, uint32_t binding, const Image& img, const ImageSampler& sampler) -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert(m_info.getImageCount() > 0);

  VkDescriptorImageInfo imgInfo = {};
  imgInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imgInfo.imageView             = img.getVkImageView();
  imgInfo.sampler               = sampler.getVkSampler();

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = m_sets[set->m_id];
  descriptorWrite.dstBinding           = binding;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pImageInfo           = &imgInfo;

  vkUpdateDescriptorSets(m_vkDevice, 1U, &descriptorWrite, 0U, nullptr);
}

auto DescriptorGroup::bindUniformBuffer(DescriptorSet* set, uint32_t binding, const Buffer& buffer)
    -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert(m_info.getUniformBufferCount() > 0);

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer                 = buffer.getVkBuffer();
  bufferInfo.offset                 = 0U;
  bufferInfo.range                  = VK_WHOLE_SIZE;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = m_sets[set->m_id];
  descriptorWrite.dstBinding           = binding;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pBufferInfo          = &bufferInfo;

  vkUpdateDescriptorSets(m_vkDevice, 1U, &descriptorWrite, 0U, nullptr);
}

auto DescriptorGroup::bindUniformBufferDynamic(
    DescriptorSet* set, uint32_t binding, const Buffer& buffer, uint32_t maxSize) -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert(m_info.getUniformBufferDynamicCount() > 0);

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer                 = buffer.getVkBuffer();
  bufferInfo.offset                 = 0U;
  bufferInfo.range                  = maxSize;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = m_sets[set->m_id];
  descriptorWrite.dstBinding           = binding;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pBufferInfo          = &bufferInfo;

  vkUpdateDescriptorSets(m_vkDevice, 1U, &descriptorWrite, 0U, nullptr);
}

auto DescriptorGroup::free(DescriptorSet* set) noexcept -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert((m_free & (1 << set->m_id)) == 0); // Check if its not freed before.

  m_free |= 1 << set->m_id; // Mark the set as free.
  set->m_group = nullptr;
}

auto DescriptorManager::getVkLayout(const DescriptorInfo& info) -> VkDescriptorSetLayout {
  if (info.getBindingCount() == 0U) {
    return nullptr;
  }

  // Attempt to get a layout from a existing group.
  for (auto& group : m_groups) {
    if (group.getInfo() == info) {
      return group.getVkLayout();
    }
  }

  // If no group has the same descriptor-info then create a new group.
  m_groups.emplace_front(m_logger, m_vkDevice, info);
  return m_groups.front().getVkLayout();
}

auto DescriptorManager::allocate(const DescriptorInfo& info) -> DescriptorSet {
  // If no actual binding points are requested then we just return a dummy DescriptorSet.
  if (info.getBindingCount() == 0U) {
    return {};
  }

  // Attempt to allocate from an existing group.
  for (auto& group : m_groups) {
    if (group.getInfo() == info) {
      auto allocation = group.allocate();
      if (allocation) {
        return std::move(*allocation);
      }
    }
  }

  // If no existing group has space then greate a new group.
  m_groups.emplace_front(m_logger, m_vkDevice, info);

  // Allocate from the new group.
  return *m_groups.front().allocate();
}

} // namespace tria::gfx::internal

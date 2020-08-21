#include "descriptor_manager.hpp"
#include "buffer.hpp"
#include "debug_utils.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "tria/math/utils.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getVkDescriptorType(DescriptorBindingKind bindingKind) noexcept {
  switch (bindingKind) {
  case DescriptorBindingKind::CombinedImageSampler:
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  case DescriptorBindingKind::UniformBuffer:
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  case DescriptorBindingKind::UniformBufferDynamic:
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  case DescriptorBindingKind::StorageBuffer:
    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  }
  assert(!"Unsupported binding kind");
  return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

[[nodiscard]] auto
createVkDescriptorSetLayout(const Device* device, const DescriptorBindings& bindings)
    -> VkDescriptorSetLayout {

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
  setLayoutBindings.reserve(bindings.size());

  for (const auto& binding : bindings) {
    VkDescriptorSetLayoutBinding storageBuffBinding = {};
    storageBuffBinding.binding                      = binding.first;
    storageBuffBinding.descriptorType               = getVkDescriptorType(binding.second);
    storageBuffBinding.descriptorCount              = 1U;
    // TODO(bastian): The stage-flags should probably be configable somehow.
    storageBuffBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    setLayoutBindings.push_back(storageBuffBinding);
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount                    = setLayoutBindings.size();
  layoutInfo.pBindings                       = setLayoutBindings.data();

  VkDescriptorSetLayout result;
  checkVkResult(vkCreateDescriptorSetLayout(device->getVkDevice(), &layoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkDescriptorPool(const Device* device, const DescriptorBindings& bindings)
    -> VkDescriptorPool {

  std::vector<VkDescriptorPoolSize> sizes;

  for (const auto& binding : bindings) {
    const auto descType = getVkDescriptorType(binding.second);
    for (auto& size : sizes) {
      if (size.type == descType) {
        size.descriptorCount += g_descriptorSetsPerGroup;
        goto NextBinding;
      }
    }
    sizes.push_back({descType, g_descriptorSetsPerGroup});
  NextBinding:
    continue;
  }

  if (sizes.empty()) {
    // TODO(bastian): Vulkan spec does not allow for empty descriptor pools, however for code
    // simplification we want to support descriptor sets without any bindings. Needs investigation
    // to see if there is a better way.
    sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U});
  }

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount              = sizes.size();
  poolInfo.pPoolSizes                 = sizes.data();
  poolInfo.maxSets                    = g_descriptorSetsPerGroup;

  VkDescriptorPool result;
  checkVkResult(vkCreateDescriptorPool(device->getVkDevice(), &poolInfo, nullptr, &result));
  return result;
}

template <unsigned int Count>
auto allocVkDescriptorSets(
    const Device* device,
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

  checkVkResult(vkAllocateDescriptorSets(device->getVkDevice(), &allocInfo, output.data()));
}

} // namespace

DescriptorSet::~DescriptorSet() noexcept {
  if (m_group) {
    m_group->free(this);
    m_group = nullptr;
  }
}

auto DescriptorSet::getVkLayout() const noexcept -> VkDescriptorSetLayout {
  return m_group->getVkLayout();
}

auto DescriptorSet::getVkDescSet() const noexcept -> VkDescriptorSet {
  return m_group->getVkDescSet(m_id);
}

auto DescriptorSet::attachBuffer(uint32_t binding, const Buffer& buffer, uint32_t size) -> void {
  m_group->attachBuffer(this, binding, buffer, size);
}

auto DescriptorSet::attachImage(uint32_t binding, const Image& img, const Sampler& sampler)
    -> void {
  m_group->attachImage(this, binding, img, sampler);
}

DescriptorGroup::DescriptorGroup(
    log::Logger* logger, const Device* device, DescriptorBindings bindings, uint32_t groupId) :
    m_logger{logger}, m_device{device}, m_bindings{std::move(bindings)}, m_groupId{groupId} {

  m_vkPool   = createVkDescriptorPool(device, m_bindings);
  m_vkLayout = createVkDescriptorSetLayout(device, m_bindings);

  DBG_DESCPOOL_NAME(device, m_vkPool, "descgroup_" + std::to_string(groupId));
  DBG_DESCLAYOUT_NAME(device, m_vkLayout, "descgroup_" + std::to_string(groupId));

  // Preallocate all the descriptor sets.
  allocVkDescriptorSets<g_descriptorSetsPerGroup>(device, m_vkPool, m_vkLayout, m_sets);

  // Give a debug name to all the sets (turns into no-op in non-debug mode).
  for (auto i = 0U; i != g_descriptorSetsPerGroup; ++i) {
    DBG_DESCSET_NAME(
        device, m_sets[i], "descgroup_" + std::to_string(groupId) + "_set_" + std::to_string(i));
  }

  // Mark the allocated sets as 'free'.
  m_free = 0U;
  for (auto i = 0U; i != g_descriptorSetsPerGroup; ++i) {
    m_free |= 1 << i;
  }

  LOG_I(
      m_logger,
      "Vulkan descriptor group allocated",
      {"id", m_groupId},
      {"setCount", g_descriptorSetsPerGroup},
      {"bindingsPerSet", m_bindings.size()});
}

DescriptorGroup::~DescriptorGroup() {
#if !defined(NDEBUG)
  // Check that all sets are properly freed.
  for (auto i = 0U; i != g_descriptorSetsPerGroup; ++i) {
    assert((m_free & (1 << i)) != 0);
  }
#endif

  vkDestroyDescriptorPool(m_device->getVkDevice(), m_vkPool, nullptr);
  vkDestroyDescriptorSetLayout(m_device->getVkDevice(), m_vkLayout, nullptr);

  LOG_I(m_logger, "Vulkan descriptor group freed", {"id", m_groupId});
}

auto DescriptorGroup::allocate() noexcept -> std::optional<DescriptorSet> {
  if (m_free == 0U) {
    return std::nullopt;
  }
  auto tz = math::countTrailingZeroes(m_free); // Find the index of the first free set.
  m_free &= ~(1U << tz);                       // Un-mark the set as free.
  return DescriptorSet{this, static_cast<int32_t>(tz)};
}

auto DescriptorGroup::attachBuffer(
    DescriptorSet* set, uint32_t binding, const Buffer& buffer, uint32_t size) -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);

  auto bindingKind = getBindingKind(binding);
  if (!bindingKind) {
    assert(!"Unknown binding");
    return;
  }

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer                 = buffer.getVkBuffer();
  bufferInfo.offset                 = 0U;
  bufferInfo.range                  = size;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = m_sets[set->m_id];
  descriptorWrite.dstBinding           = binding;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = getVkDescriptorType(*bindingKind);
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pBufferInfo          = &bufferInfo;

  vkUpdateDescriptorSets(m_device->getVkDevice(), 1U, &descriptorWrite, 0U, nullptr);
}

auto DescriptorGroup::attachImage(
    DescriptorSet* set, uint32_t binding, const Image& img, const Sampler& sampler) -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert(getBindingKind(binding) == DescriptorBindingKind::CombinedImageSampler);

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

  vkUpdateDescriptorSets(m_device->getVkDevice(), 1U, &descriptorWrite, 0U, nullptr);
}

auto DescriptorGroup::free(DescriptorSet* set) noexcept -> void {
  assert(set);
  assert(set->m_group == this);
  assert(set->m_id >= 0);
  assert((m_free & (1U << set->m_id)) == 0); // Check if its not freed before.

  m_free |= 1U << set->m_id; // Mark the set as free.
  set->m_group = nullptr;
}

auto DescriptorManager::getVkLayout(const DescriptorBindings& bindings) -> VkDescriptorSetLayout {
  // Attempt to get a layout from a existing group.
  for (auto& group : m_groups) {
    if (group.getBindings() == bindings) {
      return group.getVkLayout();
    }
  }

  // If no group has the same descriptor-info then create a new group.
  m_groups.emplace_front(m_logger, m_device, bindings, m_groupIdCounter++);
  return m_groups.front().getVkLayout();
}

auto DescriptorManager::allocate(const DescriptorBindings& bindings) -> DescriptorSet {
  // Attempt to allocate from an existing group.
  for (auto& group : m_groups) {
    if (group.getBindings() == bindings) {
      auto allocation = group.allocate();
      if (allocation) {
        return std::move(*allocation);
      }
    }
  }

  // If no existing group has space then greate a new group.
  m_groups.emplace_front(m_logger, m_device, bindings, m_groupIdCounter++);

  // Allocate from the new group.
  return *m_groups.front().allocate();
}

} // namespace tria::gfx::internal

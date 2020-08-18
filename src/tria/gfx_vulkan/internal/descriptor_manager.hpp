#pragma once
#include "tria/log/api.hpp"
#include <array>
#include <forward_list>
#include <optional>
#include <utility>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

// TODO(bastian): Needs testing to what number of sets per VkDescriptorPool makes sense.
constexpr auto g_descriptorSetsPerGroup = 6;

class Device;
class Image;
class Sampler;
class Buffer;
class DescriptorGroup;

/* Type of a binding.
 */
enum class DescriptorBindingKind {
  CombinedImageSampler = 1,
  UniformBuffer        = 2,
  UniformBufferDynamic = 3,
  StorageBuffer        = 4,
};

/* Binding id + the kind of binding.
 */
using DescriptorBinding = std::pair<uint32_t, DescriptorBindingKind>;

/* Set of bindings.
 */
using DescriptorBindings = std::vector<DescriptorBinding>;

/* Handle to an allocated VkDescriptorSet.
 * Is automatically freed in the destructor.
 */
class DescriptorSet final {
  friend DescriptorGroup;

public:
  DescriptorSet() : m_group{nullptr} {}
  DescriptorSet(const DescriptorSet& rhs) = delete;
  DescriptorSet(DescriptorSet&& rhs) noexcept {
    m_group     = rhs.m_group;
    m_id        = rhs.m_id;
    rhs.m_group = nullptr;
  };
  ~DescriptorSet() noexcept;

  auto operator=(const DescriptorSet& rhs) -> DescriptorSet& = delete;
  auto operator=(DescriptorSet&& rhs) noexcept -> DescriptorSet& {
    m_group     = rhs.m_group;
    m_id        = rhs.m_id;
    rhs.m_group = nullptr;
    return *this;
  }

  [[nodiscard]] auto getVkLayout() const noexcept -> VkDescriptorSetLayout;
  [[nodiscard]] auto getVkDescSet() const noexcept -> VkDescriptorSet;

  /* Attach a buffer (uniform (dynamic), storage) to a storage-buffer binding slot.
   */
  auto attachBuffer(uint32_t binding, const Buffer& buffer, uint32_t size) -> void;

  /* Attach an image and sampler to a combined-image-sampler binding slot.
   */
  auto attachImage(uint32_t binding, const Image& img, const Sampler& sampler) -> void;

private:
  DescriptorGroup* m_group;
  int32_t m_id;

  DescriptorSet(DescriptorGroup* group, int32_t id) noexcept : m_group{group}, m_id{id} {};
};

/* Pool where descriptors of the same layout can be allocated from.
 * Note: Api is NOT thread-safe.
 */
class DescriptorGroup final {
public:
  DescriptorGroup() = delete;
  DescriptorGroup(
      log::Logger* logger, const Device* device, DescriptorBindings bindings, uint32_t groupId);
  DescriptorGroup(const DescriptorGroup& rhs) = delete;
  DescriptorGroup(DescriptorGroup&& rhs)      = delete;
  ~DescriptorGroup();

  auto operator=(const DescriptorGroup& rhs) -> DescriptorGroup& = delete;
  auto operator=(DescriptorGroup&& rhs) -> DescriptorGroup& = delete;

  [[nodiscard]] auto getBindings() const noexcept -> const DescriptorBindings& {
    return m_bindings;
  }

  [[nodiscard]] auto getBindingKind(uint32_t binding) -> std::optional<DescriptorBindingKind> {
    for (const auto& b : m_bindings) {
      if (b.first == binding) {
        return b.second;
      }
    }
    return std::nullopt;
  }

  [[nodiscard]] auto getVkLayout() const noexcept -> VkDescriptorSetLayout { return m_vkLayout; }

  /* Lookup a VkDescriptorSet that has been allocated from this group.
   */
  [[nodiscard]] auto getVkDescSet(int32_t id) const noexcept -> VkDescriptorSet {
    return m_sets[id];
  }

  /* Allocate a new DescriptorSet.
   * Returns a null-optional if no space is left in the group.
   */
  [[nodiscard]] auto allocate() noexcept -> std::optional<DescriptorSet>;

  /* Attach a buffer to an allocated DescriptorSet.
   */
  auto attachBuffer(DescriptorSet* set, uint32_t binding, const Buffer& buffer, uint32_t size)
      -> void;

  /* Attach an image and sampler to an allocated DescriptorSet.
   */
  auto attachImage(DescriptorSet* set, uint32_t binding, const Image& img, const Sampler& sampler)
      -> void;

  /* Return a DescriptorSet to the group.
   * Note: DescriptorSet has to have been allocated from this group.
   */
  auto free(DescriptorSet* set) noexcept -> void;

private:
  tria::log::Logger* m_logger;
  const Device* m_device;
  DescriptorBindings m_bindings;
  uint32_t m_groupId;
  VkDescriptorPool m_vkPool;
  VkDescriptorSetLayout m_vkLayout;
  std::array<VkDescriptorSet, g_descriptorSetsPerGroup> m_sets;
  uint32_t m_free; // Bit mask of free descriptor sets.
};

/* DescriptorSet allocator.
 * Note: Api is NOT thread-safe.
 */
class DescriptorManager final {
public:
  DescriptorManager(log::Logger* logger, const Device* device) :
      m_logger{logger}, m_device{device}, m_groupIdCounter{0U} {}
  DescriptorManager(const DescriptorManager& rhs)     = delete;
  DescriptorManager(DescriptorManager&& rhs) noexcept = delete;
  ~DescriptorManager()                                = default;

  auto operator=(const DescriptorManager& rhs) -> DescriptorManager& = delete;
  auto operator=(DescriptorManager&& rhs) noexcept -> DescriptorManager& = delete;

  /* Get a VkDescriptorSetLayout that is compatible with the given descriptor bindings.
   */
  [[nodiscard]] auto getVkLayout(const DescriptorBindings& info) -> VkDescriptorSetLayout;

  /* Allocate a DescriptorSet that satisfies the given descriptor bindings.
   */
  [[nodiscard]] auto allocate(const DescriptorBindings& info) -> DescriptorSet;

private:
  log::Logger* m_logger;
  const Device* m_device;
  uint32_t m_groupIdCounter;
  std::forward_list<DescriptorGroup> m_groups;
};

using DescriptorManagerUnique = std::unique_ptr<DescriptorManager>;

} // namespace tria::gfx::internal

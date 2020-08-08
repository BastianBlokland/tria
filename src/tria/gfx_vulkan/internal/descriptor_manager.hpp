#pragma once
#include "tria/log/api.hpp"
#include <array>
#include <forward_list>
#include <optional>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

// TODO(bastian): Needs testing to what number of sets per VkDescriptorPool makes sense.
constexpr auto g_descriptorSetsPerGroup = 6;

class Device;
class Image;
class Sampler;
class Buffer;
class DescriptorGroup;

/* Information about the types of descriptors in a DescriptorSet.
 * Note: The order of bindings in a descriptor set is fixed:
 * 1: ImageSampler
 * 2: UniformBufferDynamic
 * Binding numbers are sequential starting from 0.
 */
class DescriptorInfo final {
public:
  DescriptorInfo() noexcept : m_imageCount{0}, m_uniformBufferDynamicCount{0} {}
  DescriptorInfo(unsigned int imageCount, unsigned int uniformBufferDynamicCount) noexcept :
      m_imageCount{imageCount}, m_uniformBufferDynamicCount{uniformBufferDynamicCount} {}

  [[nodiscard]] constexpr auto operator==(const DescriptorInfo& rhs) const noexcept -> bool {
    return m_imageCount == rhs.m_imageCount &&
        m_uniformBufferDynamicCount == rhs.m_uniformBufferDynamicCount;
  }

  [[nodiscard]] constexpr auto operator!=(const DescriptorInfo& rhs) const noexcept -> bool {
    return !operator==(rhs);
  }

  [[nodiscard]] constexpr auto getImageCount() const noexcept { return m_imageCount; }

  [[nodiscard]] constexpr auto getUniformBufferDynamicCount() const noexcept {
    return m_uniformBufferDynamicCount;
  }

  /* Total amount of bindings.
   */
  [[nodiscard]] constexpr auto getBindingCount() const noexcept {
    return m_imageCount + m_uniformBufferDynamicCount;
  }

private:
  unsigned int m_imageCount;
  unsigned int m_uniformBufferDynamicCount;
};

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

  /* Does this handle point to a VkDescriptorSet that has any binding points.
   */
  [[nodiscard]] auto hasBindingPoints() const noexcept { return m_group; }
  [[nodiscard]] auto getInfo() const noexcept -> const DescriptorInfo&;
  [[nodiscard]] auto getVkLayout() const noexcept -> VkDescriptorSetLayout;
  [[nodiscard]] auto getVkDescSet() const noexcept -> VkDescriptorSet;

  /* Attach an image and sampler to a combined-image-sampler binding slot.
   */
  auto attachImage(uint32_t binding, const Image& img, const Sampler& sampler) -> void;

  /* Attach a uniform buffer to a uniform binding slot.
   * Size of the data is static but the offset into the buffer is given dynamically at bind-time.
   */
  auto attachUniformBufferDynamic(uint32_t binding, const Buffer& buffer, uint32_t size) -> void;

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
  DescriptorGroup(log::Logger* logger, const Device* device, DescriptorInfo info, uint32_t groupId);
  DescriptorGroup(const DescriptorGroup& rhs) = delete;
  DescriptorGroup(DescriptorGroup&& rhs)      = delete;
  ~DescriptorGroup();

  auto operator=(const DescriptorGroup& rhs) -> DescriptorGroup& = delete;
  auto operator=(DescriptorGroup&& rhs) -> DescriptorGroup& = delete;

  [[nodiscard]] auto getInfo() const noexcept -> const DescriptorInfo& { return m_info; }
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

  /* Attach an image and sampler to an allocated DescriptorSet.
   */
  auto attachImage(DescriptorSet* set, uint32_t binding, const Image& img, const Sampler& sampler)
      -> void;

  /* Attach a dynamic uniform-buffer to an allocated DescriptorSet.
   */
  auto attachUniformBufferDynamic(
      DescriptorSet* set, uint32_t binding, const Buffer& buffer, uint32_t maxSize) -> void;

  /* Return a DescriptorSet to the group.
   * Note: DescriptorSet has to have been allocated from this group.
   */
  auto free(DescriptorSet* set) noexcept -> void;

private:
  tria::log::Logger* m_logger;
  const Device* m_device;
  DescriptorInfo m_info;
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

  /* Get a VkDescriptorSetLayout that is compatible to the given DescriptorInfo.
   */
  [[nodiscard]] auto getVkLayout(const DescriptorInfo& info) -> VkDescriptorSetLayout;

  /* Allocate a DescriptorSet that satisfies the given DescriptorInfo.
   */
  [[nodiscard]] auto allocate(const DescriptorInfo& info) -> DescriptorSet;

private:
  log::Logger* m_logger;
  const Device* m_device;
  uint32_t m_groupIdCounter;
  std::forward_list<DescriptorGroup> m_groups;
};

using DescriptorManagerUnique = std::unique_ptr<DescriptorManager>;

} // namespace tria::gfx::internal

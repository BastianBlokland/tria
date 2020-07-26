#pragma once
#include "buffer.hpp"
#include "device.hpp"
#include "tria/log/api.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Uniform data container.
 * Responsible for managing descriptor sets to use for high-frequency uniform data.
 */
class UniformContainer final {
public:
  UniformContainer(log::Logger* logger, Device* device);
  ~UniformContainer();

  /* Layout of the descriptor-set that data will be uploaded to.
   * Note: Provides a single binding at location 0.
   */
  [[nodiscard]] auto getVkDescLayout() const noexcept { return m_vkDescLayout; }

  /* Discard any previously uploaded data.
   * Note: Care must be taken to avoid resetting while any descriptor-set from this container is
   * still in use.
   */
  auto reset() noexcept -> void;

  /* Upload new data.
   * Returned is a handle to a descriptor-set and an offset into that descriptor-set.
   */
  auto upload(const void* data, size_t size) -> std::pair<VkDescriptorSet, uint32_t>;

private:
  struct DescData final {
    VkDescriptorSet descSet;
    Buffer buffer;
    uint32_t offset;

    DescData(VkDescriptorSet descSet, Buffer buffer, uint32_t offset) :
        descSet{descSet}, buffer{std::move(buffer)}, offset{offset} {}
  };

  log::Logger* m_logger;
  Device* m_device;
  uint32_t m_minAlignment;
  VkDescriptorSetLayout m_vkDescLayout;
  VkDescriptorPool m_vkDescPool;

  std::vector<DescData> m_sets;
};

using UniformContainerUnique = std::unique_ptr<UniformContainer>;

} // namespace tria::gfx::internal

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
  UniformContainer(const UniformContainer& rhs)     = delete;
  UniformContainer(UniformContainer&& rhs) noexcept = delete;
  ~UniformContainer()                               = default;

  auto operator=(const UniformContainer& rhs) -> UniformContainer& = delete;
  auto operator=(UniformContainer&& rhs) noexcept -> UniformContainer& = delete;

  /* Layout of the descriptor-set that data will be uploaded to.
   * Note: Provides a single binding at location 0.
   */
  [[nodiscard]] auto getVkDescLayout() const noexcept {
    return m_device->getDescManager().getVkLayout(m_descInfo);
  }

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
    DescriptorSet descSet;
    Buffer buffer;
    uint32_t offset;

    DescData(DescriptorSet descSet, Buffer buffer, uint32_t offset) :
        descSet{std::move(descSet)}, buffer{std::move(buffer)}, offset{offset} {}
  };

  log::Logger* m_logger;
  Device* m_device;
  DescriptorInfo m_descInfo;
  uint32_t m_minAlignment;

  std::vector<DescData> m_sets;
};

using UniformContainerUnique = std::unique_ptr<UniformContainer>;

} // namespace tria::gfx::internal

#pragma once
#include "device.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/log/api.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Mesh resource.
 * Holding a vertex-buffer.
 */
class Mesh final {
public:
  using AssetType = asset::Mesh;

  Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset);
  Mesh(const Mesh& rhs) = delete;
  Mesh(Mesh&& rhs)      = delete;
  ~Mesh();

  auto operator=(const Mesh& rhs) -> Mesh& = delete;
  auto operator=(Mesh&& rhs) -> Mesh& = delete;

  [[nodiscard]] auto getVertexCount() const noexcept { return m_vertexCount; }

  [[nodiscard]] auto getVkVertexBuffer() const noexcept { return m_vkVertexBuffer; }

  [[nodiscard]] auto getVkVertexBindingDescriptions() const noexcept
      -> std::vector<VkVertexInputBindingDescription>;

  [[nodiscard]] auto getVkVertexAttributeDescriptions() const noexcept
      -> std::vector<VkVertexInputAttributeDescription>;

private:
  const Device* m_device;
  uint32_t m_vertexCount;
  VkBuffer m_vkVertexBuffer;
  MemoryBlock m_vertexBufferMemory;
};

} // namespace tria::gfx::internal

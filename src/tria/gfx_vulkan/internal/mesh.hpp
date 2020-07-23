#pragma once
#include "buffer.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/log/api.hpp"
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
  ~Mesh()               = default;

  auto operator=(const Mesh& rhs) -> Mesh& = delete;
  auto operator=(Mesh&& rhs) -> Mesh& = delete;

  [[nodiscard]] auto getVertexCount() const noexcept { return m_vertexCount; }

  [[nodiscard]] auto getVertexBuffer() const noexcept -> const Buffer& { return m_vertexBuffer; }

  [[nodiscard]] auto getVkVertexBindingDescriptions() const noexcept
      -> std::vector<VkVertexInputBindingDescription>;

  [[nodiscard]] auto getVkVertexAttributeDescriptions() const noexcept
      -> std::vector<VkVertexInputAttributeDescription>;

private:
  uint32_t m_vertexCount;
  Buffer m_vertexBuffer;
};

} // namespace tria::gfx::internal

#pragma once
#include "buffer.hpp"
#include "transferer.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/log/api.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Mesh resource.
 * Holds vertex and index data.
 */
class Mesh final {
public:
  using AssetType = asset::Mesh;
  using IndexType = asset::IndexType;

  Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset);
  Mesh(const Mesh& rhs) = delete;
  Mesh(Mesh&& rhs)      = delete;
  ~Mesh()               = default;

  auto operator=(const Mesh& rhs) -> Mesh& = delete;
  auto operator=(Mesh&& rhs) -> Mesh& = delete;

  [[nodiscard]] auto getVertexCount() const noexcept { return m_asset->getVertexCount(); }
  [[nodiscard]] auto getIndexCount() const noexcept { return m_asset->getIndexCount(); }

  [[nodiscard]] auto getVkVertexBindingDescriptions() const noexcept
      -> std::vector<VkVertexInputBindingDescription>;

  [[nodiscard]] auto getVkVertexAttributeDescriptions() const noexcept
      -> std::vector<VkVertexInputAttributeDescription>;

  /* Note: Call this before accessing any resources from this mesh.
   */
  auto prepareResources(Transferer* transferer) const -> void;

  [[nodiscard]] auto getBuffer() const noexcept -> const Buffer& { return m_buffer; }
  [[nodiscard]] auto getBufferVertexOffset() const noexcept -> size_t { return 0U; }
  [[nodiscard]] auto getBufferIndexOffset() const noexcept -> size_t { return m_indexDataOffset; }

private:
  const asset::Mesh* m_asset;
  size_t m_vertexDataSize;
  size_t m_indexDataSize;
  size_t m_indexDataOffset;
  mutable bool m_bufferUploaded;
  Buffer m_buffer;
};

} // namespace tria::gfx::internal

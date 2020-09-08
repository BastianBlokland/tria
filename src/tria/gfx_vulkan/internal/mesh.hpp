#pragma once
#include "buffer.hpp"
#include "transferer.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/log/api.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Needs to match the layout of the MeshMeta structure in GLSL.
 */
struct alignas(16) MeshMeta final {
  math::Vec3f posBoundsMin;
  float padding1;
  math::Vec3f posBoundsSize;
  float padding2;
};

/* Needs to match the layout of the vertex structure in GLSL.
 */
struct alignas(16) DeviceVertex final {
  math::Vec<uint16_t, 3> posFrac; // 0 - 1 fraction of the mesh bounds, 16 bit floats.
  uint16_t texcoordX;             // 16 bit float.
  math::Vec<uint16_t, 3> nrm;     // 16 bit floats.
  uint16_t texcoordY;             // 16 bit float.
  math::Vec<uint16_t, 3> tan;     // 16 bit floats.
  uint16_t biTanSign;             // 16 bit float.
};

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

  /* Note: Call this before accessing any resources from this mesh.
   */
  auto prepareResources(Transferer* transferer) const -> void;

  [[nodiscard]] auto getVertexBuffer() const noexcept -> const Buffer& { return m_vertexBuffer; }
  [[nodiscard]] auto getIndexBuffer() const noexcept -> const Buffer& { return m_indexBuffer; }

private:
  const asset::Mesh* m_asset;
  size_t m_vertexDataSize;
  size_t m_indexDataSize;
  mutable bool m_buffersUploaded;
  Buffer m_vertexBuffer;
  Buffer m_indexBuffer;
};

} // namespace tria::gfx::internal

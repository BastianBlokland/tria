#include "mesh.hpp"
#include "debug_utils.hpp"
#include "tria/math/utils.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_asset{asset}, m_buffersUploaded{false} {

  assert(device);
  assert(m_asset);

  m_vertexDataSize = sizeof(MeshMeta) + sizeof(DeviceVertex) * m_asset->getVertexCount();
  m_indexDataSize  = sizeof(IndexType) * m_asset->getIndexCount();

  m_vertexBuffer =
      Buffer{device, m_vertexDataSize, MemoryLocation::Device, BufferUsage::DeviceStorageData};
  DBG_BUFFER_NAME(device, m_vertexBuffer.getVkBuffer(), "vertex_" + asset->getId());

  m_indexBuffer =
      Buffer{device, m_indexDataSize, MemoryLocation::Device, BufferUsage::DeviceIndexData};
  DBG_BUFFER_NAME(device, m_indexBuffer.getVkBuffer(), "index_" + asset->getId());

  LOG_D(
      logger,
      "Vulkan mesh created",
      {"asset", m_asset->getId()},
      {"vertices", m_asset->getVertexCount()},
      {"indices", m_asset->getIndexCount()},
      {"vertexMemory", log::MemSize{m_vertexBuffer.getSize()}},
      {"indexMemory", log::MemSize{m_indexBuffer.getSize()}});
}

auto Mesh::prepareResources(Transferer* transferer) const -> void {
  if (!m_buffersUploaded) {

    auto meshData = math::RawData{m_vertexDataSize};

    // Mesh meta.
    auto* meshMeta          = reinterpret_cast<MeshMeta*>(meshData.begin());
    meshMeta->posBoundsMin  = m_asset->getPosBounds().min();
    meshMeta->posBoundsSize = m_asset->getPosBounds().max() - m_asset->getPosBounds().min();

    // Vertex data.
    auto* devItr = reinterpret_cast<DeviceVertex*>(meshData.begin() + sizeof(MeshMeta));
    for (auto itr = m_asset->getVertexBegin(); itr != m_asset->getVertexEnd(); ++itr, ++devItr) {
      // 0 - 1 fraction of the position inside the mesh bounds.
      devItr->posFrac.x() = math::floatToHalf(
          (itr->position.x() - meshMeta->posBoundsMin.x()) / meshMeta->posBoundsSize.x());
      devItr->posFrac.y() = math::floatToHalf(
          (itr->position.y() - meshMeta->posBoundsMin.y()) / meshMeta->posBoundsSize.y());
      devItr->posFrac.z() = math::floatToHalf(
          (itr->position.z() - meshMeta->posBoundsMin.z()) / meshMeta->posBoundsSize.z());

      devItr->texcoordX = math::floatToHalf(itr->texcoord.x());
      devItr->texcoordY = math::floatToHalf(itr->texcoord.y());

      devItr->nrm.x() = math::floatToHalf(itr->normal.x());
      devItr->nrm.y() = math::floatToHalf(itr->normal.y());
      devItr->nrm.z() = math::floatToHalf(itr->normal.z());

      devItr->tan.x()   = math::floatToHalf(itr->tangent.x());
      devItr->tan.y()   = math::floatToHalf(itr->tangent.y());
      devItr->tan.z()   = math::floatToHalf(itr->tangent.z());
      devItr->biTanSign = math::floatToHalf(itr->tangent.w());
    }

    // Mesh data.
    transferer->queueTransfer(meshData.begin(), m_vertexBuffer, 0U, m_vertexDataSize);

    // Index data.
    transferer->queueTransfer(m_asset->getIndexBegin(), m_indexBuffer, 0U, m_indexDataSize);

    m_buffersUploaded = true;
  }
}

} // namespace tria::gfx::internal

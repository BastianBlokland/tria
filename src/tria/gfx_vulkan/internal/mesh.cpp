#include "mesh.hpp"
#include "debug_utils.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_asset{asset}, m_buffersUploaded{false} {

  assert(device);
  assert(m_asset);

  m_vertexDataSize = sizeof(DeviceVertex) * m_asset->getVertexCount();
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

    // Vertex data.
    // We convert the asset vertex data into a more packed format.
    auto deviceVertexData = math::PodVector<DeviceVertex>(m_asset->getVertexCount());
    auto devItr           = deviceVertexData.begin();
    for (auto itr = m_asset->getVertexBegin(); itr != m_asset->getVertexEnd(); ++itr, ++devItr) {
      devItr->pos       = itr->position;
      devItr->texcoordX = itr->texcoord.x();
      // TODO(bastian): We could pack the normal data using 8 bit per channel.
      devItr->nrm       = itr->normal;
      devItr->texcoordY = itr->texcoord.y();
    }
    transferer->queueTransfer(deviceVertexData.begin(), m_vertexBuffer, 0U, m_vertexDataSize);

    // Index data.
    transferer->queueTransfer(m_asset->getIndexBegin(), m_indexBuffer, 0U, m_indexDataSize);

    m_buffersUploaded = true;
  }
}

} // namespace tria::gfx::internal

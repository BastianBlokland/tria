#include "mesh.hpp"
#include "debug_utils.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_asset{asset}, m_bufferUploaded{false} {

  assert(device);
  assert(m_asset);

  m_vertexDataSize  = sizeof(asset::Vertex) * m_asset->getVertexCount();
  m_indexDataSize   = sizeof(IndexType) * m_asset->getIndexCount();
  m_indexDataOffset = m_vertexDataSize + padToAlignment(m_vertexDataSize, sizeof(IndexType));

  m_buffer = Buffer{device,
                    m_indexDataOffset + m_indexDataSize,
                    MemoryLocation::Device,
                    BufferUsage::DeviceVertexAndIndexData};
  DBG_BUFFER_NAME(device, m_buffer.getVkBuffer(), asset->getId());

  LOG_D(
      logger,
      "Vulkan mesh created",
      {"asset", m_asset->getId()},
      {"vertices", m_asset->getVertexCount()},
      {"indices", m_asset->getIndexCount()},
      {"memory", log::MemSize{m_buffer.getSize()}});
}

auto Mesh::prepareResources(Transferer* transferer) const -> void {
  if (!m_bufferUploaded) {

    // Vertex data.
    transferer->queueTransfer(m_asset->getVertexBegin(), m_buffer, 0U, m_vertexDataSize);

    // Index data.
    transferer->queueTransfer(
        m_asset->getIndexBegin(), m_buffer, m_indexDataOffset, m_indexDataSize);

    m_bufferUploaded = true;
  }
}

auto Mesh::getVkVertexBindingDescriptions() const noexcept
    -> std::vector<VkVertexInputBindingDescription> {

  auto result         = std::vector<VkVertexInputBindingDescription>{1};
  result[0].binding   = 0;
  result[0].stride    = sizeof(asset::Vertex);
  result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return result;
}

auto Mesh::getVkVertexAttributeDescriptions() const noexcept
    -> std::vector<VkVertexInputAttributeDescription> {
  static_assert(
      sizeof(asset::Vertex::position) == sizeof(float) * 3, "Unexpected vertex position size");
  static_assert(
      sizeof(asset::Vertex::normal) == sizeof(float) * 3, "Unexpected vertex normal size");
  static_assert(sizeof(asset::Vertex::color) == sizeof(float) * 4, "Unexpected vertex color size");
  static_assert(
      sizeof(asset::Vertex::texcoord) == sizeof(float) * 2, "Unexpected vertex texcoord size");

  auto result = std::vector<VkVertexInputAttributeDescription>{4};

  // Position.
  result[0].binding  = 0;
  result[0].location = 0;
  result[0].format   = VK_FORMAT_R32G32B32_SFLOAT; // vec3.
  result[0].offset   = offsetof(asset::Vertex, position);

  // Normal.
  result[1].binding  = 0;
  result[1].location = 1;
  result[1].format   = VK_FORMAT_R32G32B32_SFLOAT; // vec3.
  result[1].offset   = offsetof(asset::Vertex, normal);

  // Color.
  result[2].binding  = 0;
  result[2].location = 2;
  result[2].format   = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4.
  result[2].offset   = offsetof(asset::Vertex, color);

  // Uv.
  result[3].binding  = 0;
  result[3].location = 3;
  result[3].format   = VK_FORMAT_R32G32_SFLOAT; // vec2.
  result[3].offset   = offsetof(asset::Vertex, texcoord);

  return result;
}

} // namespace tria::gfx::internal

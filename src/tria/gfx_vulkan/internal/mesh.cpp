#include "mesh.hpp"
#include "tria/asset/mesh.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_asset{asset}, m_buffersUploaded{false} {

  assert(device);
  assert(asset);

  m_vertexBuffer = Buffer{device,
                          sizeof(asset::Vertex) * asset->getVertexCount(),
                          MemoryLocation::Device,
                          BufferUsage::VertexData};

  LOG_D(
      logger,
      "Vulkan mesh created",
      {"asset", asset->getId()},
      {"vertices", asset->getVertexCount()},
      {"vertexMemory", log::MemSize{m_vertexBuffer.getSize()}});
}

auto Mesh::transferData(Transferer* transferer) const noexcept -> void {
  if (!m_buffersUploaded) {
    transferer->queueTransfer(
        m_asset->getVertexBegin(),
        sizeof(asset::Vertex) * m_asset->getVertexCount(),
        m_vertexBuffer);

    m_buffersUploaded = true;
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
  static_assert(sizeof(asset::Vertex::color) == sizeof(float) * 4, "Unexpected vertex color size");

  auto result = std::vector<VkVertexInputAttributeDescription>{2};

  // Position.
  result[0].binding  = 0;
  result[0].location = 0;
  result[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
  result[0].offset   = offsetof(asset::Vertex, position);

  // Color.
  result[1].binding  = 0;
  result[1].location = 1;
  result[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
  result[1].offset   = offsetof(asset::Vertex, color);

  return result;
}

} // namespace tria::gfx::internal

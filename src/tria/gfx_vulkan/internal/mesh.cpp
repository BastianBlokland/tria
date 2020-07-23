#include "mesh.hpp"
#include "tria/asset/mesh.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_vertexCount{static_cast<uint32_t>(asset->getVertCount())} {
  assert(device);
  assert(m_vertexCount > 0);
  assert(asset);

  auto size      = sizeof(asset::Vertex) * m_vertexCount;
  m_vertexBuffer = Buffer{device, size, MemoryLocation::Host, BufferUsage::VertexData};
  m_vertexBuffer.upload(asset->getVertBegin(), size);

  LOG_D(
      logger,
      "Vulkan mesh created",
      {"asset", asset->getId()},
      {"vertices", m_vertexCount},
      {"vertexMemory", log::MemSize{size}});
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

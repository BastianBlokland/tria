#include "mesh.hpp"
#include "tria/asset/mesh.hpp"
#include "utils.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getVkMemoryRequirements(VkDevice vkDevice, VkBuffer vkBuffer)
    -> VkMemoryRequirements {
  VkMemoryRequirements result;
  vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &result);
  return result;
}

[[nodiscard]] auto createVkVertexBuffer(VkDevice vkDevice, uint32_t size) -> VkBuffer {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size        = size;
  bufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer result;
  checkVkResult(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &result));
  return result;
}

} // namespace

Mesh::Mesh(log::Logger* logger, Device* device, const asset::Mesh* asset) :
    m_device{device}, m_vertexCount{static_cast<uint32_t>(asset->getVertCount())} {
  assert(m_device);
  assert(m_vertexCount > 0);
  assert(asset);

  // Create a vertex buffer.
  m_vkVertexBuffer =
      createVkVertexBuffer(m_device->getVkDevice(), sizeof(asset::Vertex) * m_vertexCount);

  // Allocate memory for it.
  const auto memRequirements = getVkMemoryRequirements(m_device->getVkDevice(), m_vkVertexBuffer);
  m_vertexBufferMemory       = device->getMemory().allocate(MemoryLocation::Host, memRequirements);

  // Bind the memory to the buffer.
  m_vertexBufferMemory.bindToBuffer(m_vkVertexBuffer);

  // Copy the vertex data to the buffer.
  std::memcpy(
      m_vertexBufferMemory.getMappedPtr(),
      asset->getVertBegin(),
      m_vertexCount * sizeof(asset::Vertex));
  m_vertexBufferMemory.flush();

  LOG_D(
      logger,
      "Vulkan mesh created",
      {"asset", asset->getId()},
      {"vertices", m_vertexCount},
      {"vertexMemory", log::MemSize{memRequirements.size}});
}

Mesh::~Mesh() { vkDestroyBuffer(m_device->getVkDevice(), m_vkVertexBuffer, nullptr); }

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

#include "image.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto getVkMemoryRequirements(VkDevice vkDevice, VkImage vkImage)
    -> VkMemoryRequirements {
  VkMemoryRequirements result;
  vkGetImageMemoryRequirements(vkDevice, vkImage, &result);
  return result;
}

[[nodiscard]] auto createVkImage(VkDevice vkDevice, ImageSize size, VkFormat vkFormat) -> VkImage {
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType         = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width      = static_cast<uint32_t>(size.x());
  imageInfo.extent.height     = static_cast<uint32_t>(size.y());
  imageInfo.extent.depth      = 1U;
  imageInfo.mipLevels         = 1U;
  imageInfo.arrayLayers       = 1U;
  imageInfo.format            = vkFormat;
  imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage             = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;

  VkImage result;
  checkVkResult(vkCreateImage(vkDevice, &imageInfo, nullptr, &result));
  return result;
}

} // namespace

Image::Image(Device* device, ImageSize size, VkFormat vkFormat) :
    m_device{device}, m_size{size}, m_vkFormat{vkFormat} {

  assert(m_device);

  // Create an image.
  m_vkImage = createVkImage(device->getVkDevice(), size, vkFormat);

  // Allocate device memory for it.
  auto memoryRequirements = getVkMemoryRequirements(device->getVkDevice(), m_vkImage);
  m_memory = device->getMemory().allocate(MemoryLocation::Device, memoryRequirements);

  // Bind the memory to the image.
  m_memory.bindToImage(m_vkImage);
}

Image::~Image() {
  if (m_vkImage) {
    vkDestroyImage(m_device->getVkDevice(), m_vkImage, nullptr);
  }
  // Note: Memory is reclaimed by the destructor of MemoryBlock.
}

} // namespace tria::gfx::internal

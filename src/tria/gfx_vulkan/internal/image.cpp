#include "image.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto calcMipLevels(ImageSize size) noexcept -> uint32_t {
  // Check how many times we can cut the image in half before both sides hit 1 pixel.
  const auto biggestSide = std::max(size.x(), size.y());
  const auto mips        = std::log2(biggestSide);
  return static_cast<uint32_t>(std::floor(mips)) + 1U; // +1 to include the 'base' image.
}

[[nodiscard]] auto getVkMemoryRequirements(VkDevice vkDevice, VkImage vkImage)
    -> VkMemoryRequirements {
  VkMemoryRequirements result;
  vkGetImageMemoryRequirements(vkDevice, vkImage, &result);
  return result;
}

[[nodiscard]] auto createVkImage(
    VkDevice vkDevice, ImageSize size, VkFormat vkFormat, uint32_t mipLevels, bool genMipMaps)
    -> VkImage {
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType         = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width      = static_cast<uint32_t>(size.x());
  imageInfo.extent.height     = static_cast<uint32_t>(size.y());
  imageInfo.extent.depth      = 1U;
  imageInfo.mipLevels         = mipLevels;
  imageInfo.arrayLayers       = 1U;
  imageInfo.format            = vkFormat;
  imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage             = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  if (genMipMaps) {
    // To generate mip-maps we copy from the base image, so we need to transfer from it.
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;

  VkImage result;
  checkVkResult(vkCreateImage(vkDevice, &imageInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto
createVkImageView(VkDevice vkDevice, VkImage image, VkFormat format, uint32_t mipLevels)
    -> VkImageView {
  VkImageViewCreateInfo createInfo           = {};
  createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image                           = image;
  createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format                          = format;
  createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  createInfo.subresourceRange.baseMipLevel   = 0U;
  createInfo.subresourceRange.levelCount     = mipLevels;
  createInfo.subresourceRange.baseArrayLayer = 0U;
  createInfo.subresourceRange.layerCount     = 1U;

  VkImageView result;
  checkVkResult(vkCreateImageView(vkDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

Image::Image(Device* device, ImageSize size, VkFormat vkFormat, ImageMipMode mipMode) :
    m_device{device}, m_size{size}, m_vkFormat{vkFormat}, m_mipMode{mipMode} {

  assert(m_device);

  const auto genMipMaps = mipMode == ImageMipMode::Generate;
  m_mipLevels           = genMipMaps ? calcMipLevels(size) : 1U;

  // Create an image.
  m_vkImage = createVkImage(device->getVkDevice(), size, vkFormat, m_mipLevels, genMipMaps);

  // Allocate device memory for it.
  auto memoryRequirements = getVkMemoryRequirements(device->getVkDevice(), m_vkImage);
  m_memory                = device->getMemory().allocate(
      MemoryLocation::Device, MemoryAccessType::NonLinear, memoryRequirements);

  // Bind the memory to the image.
  m_memory.bindToImage(m_vkImage);

  // Create a view over the image.
  m_vkImageView = createVkImageView(device->getVkDevice(), m_vkImage, vkFormat, m_mipLevels);
}

Image::~Image() {
  if (m_vkImage) {
    vkDestroyImage(m_device->getVkDevice(), m_vkImage, nullptr);
    vkDestroyImageView(m_device->getVkDevice(), m_vkImageView, nullptr);
  }
  // Note: Memory is reclaimed by the destructor of MemoryBlock.
}

} // namespace tria::gfx::internal

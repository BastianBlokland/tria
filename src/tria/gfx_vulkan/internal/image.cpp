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
    VkDevice vkDevice,
    ImageSize size,
    VkFormat vkFormat,
    VkImageUsageFlags imgUsages,
    VkSampleCount sampleCount,
    uint32_t mipLevels) -> VkImage {

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
  imageInfo.usage             = imgUsages;
  imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples           = sampleCount;

  VkImage result;
  checkVkResult(vkCreateImage(vkDevice, &imageInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkImageView(
    VkDevice vkDevice,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspect,
    uint32_t mipLevels) -> VkImageView {
  VkImageViewCreateInfo createInfo           = {};
  createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image                           = image;
  createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format                          = format;
  createInfo.subresourceRange.aspectMask     = aspect;
  createInfo.subresourceRange.baseMipLevel   = 0U;
  createInfo.subresourceRange.levelCount     = mipLevels;
  createInfo.subresourceRange.baseArrayLayer = 0U;
  createInfo.subresourceRange.layerCount     = 1U;

  VkImageView result;
  checkVkResult(vkCreateImageView(vkDevice, &createInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto getVkImageAspect(ImageType type) noexcept -> VkImageAspectFlags {
  switch (type) {
  case ImageType::ColorSource:
  case ImageType::ColorAttachment:
  case ImageType::Swapchain:
    return VK_IMAGE_ASPECT_COLOR_BIT;
  case ImageType::DepthAttachment:
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  return {};
}

[[nodiscard]] auto getVkImageUsage(ImageType type, bool generatedMipMaps) noexcept
    -> VkImageUsageFlags {
  switch (type) {
  case ImageType::ColorSource: {
    // Assume that we will be uploading image data from the cpu side.
    auto usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (generatedMipMaps) {
      // To generate mip-maps we copy from the base image, so we need to transfer from it.
      usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    return usage;
  }
  case ImageType::ColorAttachment:
    assert(!generatedMipMaps); // Attachments with genenerated mipmaps don't make sense.
    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  case ImageType::DepthAttachment:
    assert(!generatedMipMaps); // Attachments with genenerated mipmaps don't make sense.
    return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  case ImageType::Swapchain:
    assert(false); // Swapchain images cannot be created manually.
    return {};
  }
  return {};
}

} // namespace

Image::Image(
    Device* device,
    ImageSize size,
    VkFormat vkFormat,
    ImageType type,
    VkSampleCount sampleCount,
    ImageMipMode mipMode) :
    m_device{device}, m_size{size}, m_vkFormat{vkFormat}, m_type{type}, m_mipMode{mipMode} {

  assert(m_device);
  assert(type != ImageType::ColorAttachment || getVkFormatChannelCount(m_vkFormat) == 4U);
  assert(type != ImageType::DepthAttachment || getVkFormatChannelCount(m_vkFormat) == 1U);

  const auto genMipMaps = mipMode == ImageMipMode::Generate;
  m_mipLevels           = genMipMaps ? calcMipLevels(size) : 1U;

  const auto imgAspect = getVkImageAspect(type);
  const auto imgUsages = getVkImageUsage(type, genMipMaps);

  // Create an image.
  m_vkImage =
      createVkImage(device->getVkDevice(), size, vkFormat, imgUsages, sampleCount, m_mipLevels);

  // Allocate device memory for it.
  auto memoryRequirements = getVkMemoryRequirements(device->getVkDevice(), m_vkImage);
  m_memory                = device->getMemory().allocate(
      MemoryLocation::Device, MemoryAccessType::NonLinear, memoryRequirements);

  // Bind the memory to the image.
  m_memory.bindToImage(m_vkImage);

  // Create a view over the image.
  m_vkImageView =
      createVkImageView(device->getVkDevice(), m_vkImage, vkFormat, imgAspect, m_mipLevels);
}

Image::Image(
    const Device* device, VkImage vkImage, ImageSize size, VkFormat vkFormat, ImageType type) :
    m_device{device}, m_size{size}, m_vkFormat{vkFormat}, m_type{type}, m_vkImage{vkImage} {

  assert(m_device);

  // Existing images are assumed to have no mipmaps,
  m_mipLevels = 1U;

  const auto imgAspect = getVkImageAspect(type);

  // Create a view over the image.
  m_vkImageView =
      createVkImageView(device->getVkDevice(), m_vkImage, vkFormat, imgAspect, m_mipLevels);
}

Image::~Image() {
  if (m_vkImage && m_type != ImageType::Swapchain) {
    vkDestroyImage(m_device->getVkDevice(), m_vkImage, nullptr);
  }
  if (m_vkImageView) {
    vkDestroyImageView(m_device->getVkDevice(), m_vkImageView, nullptr);
  }
  // Note: Memory is reclaimed by the destructor of MemoryBlock.
}

} // namespace tria::gfx::internal

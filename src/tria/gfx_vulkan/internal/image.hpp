#pragma once
#include "device.hpp"
#include "utils.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

using ImageSize     = math::Vec<uint16_t, 2>;
using VkSampleCount = VkSampleCountFlagBits;

enum class ImageType {
  ColorSource,
  ColorAttachment,
  DepthAttachment,
  Swapchain,
};

enum class ImageMipMode {
  None,
  Generate,
};

/*
 * Handle to a image resource on the gpu.
 */
class Image final {
public:
  Image() : m_vkImage{nullptr}, m_vkImageView{nullptr} {}
  Image(
      Device* device,
      ImageSize size,
      VkFormat vkFormat,
      ImageType type,
      VkSampleCount sampleCount,
      ImageMipMode mipMode);
  Image(const Device* device, VkImage vkImage, ImageSize size, VkFormat vkFormat, ImageType type);
  Image(const Image& rhs) = delete;
  Image(Image&& rhs) noexcept {
    m_device          = rhs.m_device;
    m_size            = rhs.m_size;
    m_vkFormat        = rhs.m_vkFormat;
    m_type            = rhs.m_type;
    m_mipMode         = rhs.m_mipMode;
    m_mipLevels       = rhs.m_mipLevels;
    m_vkImage         = rhs.m_vkImage;
    m_vkImageView     = rhs.m_vkImageView;
    m_memory          = std::move(rhs.m_memory);
    rhs.m_vkImage     = nullptr;
    rhs.m_vkImageView = nullptr;
  }
  ~Image();

  auto operator=(const Image& rhs) -> Image& = delete;

  auto operator=(Image&& rhs) noexcept -> Image& {
    m_device          = rhs.m_device;
    m_size            = rhs.m_size;
    m_vkFormat        = rhs.m_vkFormat;
    m_type            = rhs.m_type;
    m_mipMode         = rhs.m_mipMode;
    m_mipLevels       = rhs.m_mipLevels;
    m_vkImage         = rhs.m_vkImage;
    m_vkImageView     = rhs.m_vkImageView;
    m_memory          = std::move(rhs.m_memory);
    rhs.m_vkImage     = nullptr;
    rhs.m_vkImageView = nullptr;
    return *this;
  }

  [[nodiscard]] auto getVkImage() const noexcept { return m_vkImage; }
  [[nodiscard]] auto getVkImageView() const noexcept { return m_vkImageView; }
  [[nodiscard]] auto getVkFormat() const noexcept { return m_vkFormat; }
  [[nodiscard]] auto getChannelCount() const noexcept {
    return getVkFormatChannelCount(m_vkFormat);
  }

  [[nodiscard]] auto getSize() const noexcept { return m_size; }
  [[nodiscard]] auto getPixelCount() const noexcept {
    return static_cast<uint32_t>(m_size.x()) * m_size.y();
  }

  [[nodiscard]] auto getDataSize() const noexcept {
    return getPixelCount() * getVkFormatSize(m_vkFormat);
  }
  [[nodiscard]] auto getMemSize() const noexcept { return m_memory.getSize(); }

  [[nodiscard]] auto getType() const noexcept { return m_type; }

  [[nodiscard]] auto getMipMode() const noexcept { return m_mipMode; }
  [[nodiscard]] auto getMipLevels() const noexcept { return m_mipLevels; }

private:
  const Device* m_device;
  ImageSize m_size;
  VkFormat m_vkFormat;
  ImageType m_type;
  ImageMipMode m_mipMode;
  uint32_t m_mipLevels;
  VkImage m_vkImage;
  VkImageView m_vkImageView;
  MemoryBlock m_memory;
};

[[nodiscard]] constexpr auto getName(ImageType type) noexcept -> std::string_view {
  switch (type) {
  case ImageType::ColorSource:
    return "color-source";
  case ImageType::ColorAttachment:
    return "color-attachment";
  case ImageType::DepthAttachment:
    return "depth-attachment";
  case ImageType::Swapchain:
    return "swapchain";
  }
  return "unknown";
}

[[nodiscard]] constexpr auto getName(ImageMipMode mode) noexcept -> std::string_view {
  switch (mode) {
  case ImageMipMode::None:
    return "none";
  case ImageMipMode::Generate:
    return "generate";
  }
  return "unknown";
}

} // namespace tria::gfx::internal

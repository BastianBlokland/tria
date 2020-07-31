#pragma once
#include "device.hpp"
#include "utils.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

using ImageSize = math::Vec<uint16_t, 2>;

/*
 * Handle to a image resource on the gpu.
 */
class Image final {
public:
  Image() = default;
  Image(Device* device, ImageSize size, VkFormat vkFormat);
  Image(const Image& rhs) = delete;
  Image(Image&& rhs) noexcept {
    m_device          = rhs.m_device;
    m_size            = rhs.m_size;
    m_vkFormat        = rhs.m_vkFormat;
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
  [[nodiscard]] auto getSize() const noexcept { return m_size; }
  [[nodiscard]] auto getPixelCount() const noexcept -> uint32_t {
    return static_cast<uint32_t>(m_size.x()) * m_size.y();
  }
  [[nodiscard]] auto getChannelCount() const noexcept {
    return getVkFormatChannelCount(m_vkFormat);
  }
  [[nodiscard]] auto getDataSize() const noexcept -> uint32_t {
    return getPixelCount() * getVkFormatSize(m_vkFormat);
  }
  [[nodiscard]] auto getMemSize() const noexcept { return m_memory.getSize(); }

private:
  const Device* m_device;
  ImageSize m_size;
  VkFormat m_vkFormat;
  VkImage m_vkImage;
  VkImageView m_vkImageView;
  MemoryBlock m_memory;
};

} // namespace tria::gfx::internal

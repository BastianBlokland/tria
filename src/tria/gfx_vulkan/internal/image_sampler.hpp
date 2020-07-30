#pragma once
#include "device.hpp"
#include "utils.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/*
 * Handle to a image resource on the gpu.
 */
class ImageSampler final {
public:
  ImageSampler() = default;
  ImageSampler(const Device* device);
  ImageSampler(const ImageSampler& rhs) = delete;
  ImageSampler(ImageSampler&& rhs) noexcept {
    m_device        = rhs.m_device;
    m_vkSampler     = rhs.m_vkSampler;
    rhs.m_vkSampler = nullptr;
  }
  ~ImageSampler();

  auto operator=(const ImageSampler& rhs) -> ImageSampler& = delete;

  auto operator=(ImageSampler&& rhs) noexcept -> ImageSampler& {
    m_device        = rhs.m_device;
    m_vkSampler     = rhs.m_vkSampler;
    rhs.m_vkSampler = nullptr;
    return *this;
  }

  [[nodiscard]] auto getVkSampler() const noexcept { return m_vkSampler; }

private:
  const Device* m_device;
  VkSampler m_vkSampler;
};

} // namespace tria::gfx::internal

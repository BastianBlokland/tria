#pragma once
#include "device.hpp"
#include "utils.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

enum class SamplerFilterMode : uint8_t {
  Nearest = 0,
  Linear  = 1,
};

enum class SamplerAnisotropyMode : uint8_t {
  None = 0,
  X2   = 1,
  X4   = 2,
  X8   = 3,
  X16  = 4,
};

/*
 * Handle to a sampler resource on the gpu.
 */
class Sampler final {
public:
  Sampler() = default;
  Sampler(
      const Device* device,
      SamplerFilterMode filterMode,
      SamplerAnisotropyMode anisoMode,
      uint32_t mipLevels);
  Sampler(const Sampler& rhs) = delete;
  Sampler(Sampler&& rhs) noexcept {
    m_device        = rhs.m_device;
    m_vkSampler     = rhs.m_vkSampler;
    rhs.m_vkSampler = nullptr;
  }
  ~Sampler();

  auto operator=(const Sampler& rhs) -> Sampler& = delete;

  auto operator=(Sampler&& rhs) noexcept -> Sampler& {
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

#include "sampler.hpp"

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkSampler(
    const Device* device,
    SamplerWrapMode wrapMode,
    SamplerFilterMode filterMode,
    SamplerAnisotropyMode anisoMode,
    uint32_t mipLevels) -> VkSampler {
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  switch (filterMode) {
  case SamplerFilterMode::Nearest:
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    break;
  case SamplerFilterMode::Linear:
  default:
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    break;
  }
  switch (wrapMode) {
  case SamplerWrapMode::Repeat:
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    break;
  case SamplerWrapMode::Clamp:
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    break;
  }
  if (device->getFeatures().samplerAnisotropy) {
    samplerInfo.anisotropyEnable = anisoMode != SamplerAnisotropyMode::None;
    switch (anisoMode) {
    case SamplerAnisotropyMode::X2:
      samplerInfo.maxAnisotropy = 2.0f;
      break;
    case SamplerAnisotropyMode::X4:
      samplerInfo.maxAnisotropy = 4.0f;
      break;
    case SamplerAnisotropyMode::X8:
      samplerInfo.maxAnisotropy = 8.0f;
      break;
    case SamplerAnisotropyMode::X16:
      samplerInfo.maxAnisotropy = 16.0f;
      break;
    default:
      samplerInfo.maxAnisotropy = 1.0f;
      break;
    }
  } else {
    // Anisotropic filtering not supported on this device.
    samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy    = 1.0f;
  }
  samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.compareEnable           = false;
  samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias              = 0.0f;
  samplerInfo.minLod                  = 0.0f;
  samplerInfo.maxLod                  = mipLevels;

  VkSampler result;
  checkVkResult(vkCreateSampler(device->getVkDevice(), &samplerInfo, nullptr, &result));
  return result;
}

} // namespace

Sampler::Sampler(
    const Device* device,
    SamplerWrapMode wrapMode,
    SamplerFilterMode filterMode,
    SamplerAnisotropyMode anisoMode,
    uint32_t mipLevels) :
    m_device{device} {
  m_vkSampler = createVkSampler(device, wrapMode, filterMode, anisoMode, mipLevels);
}

Sampler::~Sampler() {
  if (m_vkSampler) {
    vkDestroySampler(m_device->getVkDevice(), m_vkSampler, nullptr);
  }
}

} // namespace tria::gfx::internal

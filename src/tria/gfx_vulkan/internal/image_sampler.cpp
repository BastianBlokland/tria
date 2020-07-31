#include "image_sampler.hpp"

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkSampler(const Device* device) -> VkSampler {
  VkSamplerCreateInfo samplerInfo     = {};
  samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter               = VK_FILTER_LINEAR;
  samplerInfo.minFilter               = VK_FILTER_LINEAR;
  samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable        = false;
  samplerInfo.maxAnisotropy           = 1.0f;
  samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.compareEnable           = false;
  samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias              = 0.0f;
  samplerInfo.minLod                  = 0.0f;
  samplerInfo.maxLod                  = 0.0f;

  VkSampler result;
  checkVkResult(vkCreateSampler(device->getVkDevice(), &samplerInfo, nullptr, &result));
  return result;
}

} // namespace

ImageSampler::ImageSampler(const Device* device) : m_device{device} {
  m_vkSampler = createVkSampler(device);
}

ImageSampler::~ImageSampler() {
  if (m_vkSampler) {
    vkDestroySampler(m_device->getVkDevice(), m_vkSampler, nullptr);
  }
}

} // namespace tria::gfx::internal

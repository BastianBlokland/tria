#include "shader_asset.hpp"
#include "utils.hpp"
#include <cstdint>
#include <stdexcept>

namespace tria::gfx::internal {

ShaderAsset::ShaderAsset(const Device* device, const RawAsset& rawAsset) : m_device{device} {
  if (!m_device) {
    throw std::invalid_argument{"Device pointer cannot be null"};
  }
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize                 = rawAsset.getSize();
  createInfo.pCode                    = reinterpret_cast<const uint32_t*>(rawAsset.getData());
  checkVkResult(vkCreateShaderModule(device->getVkDevice(), &createInfo, nullptr, &m_module));
}

ShaderAsset::~ShaderAsset() { vkDestroyShaderModule(m_device->getVkDevice(), m_module, nullptr); }

[[nodiscard]] auto loadShaderAsset(const Device* device, const fs::path& path) -> ShaderAssetPtr {
  auto rawAsset = loadRawAsset(path);
  return std::make_unique<ShaderAsset>(device, *rawAsset);
}

} // namespace tria::gfx::internal

#include "shader.hpp"
#include "device.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createShaderModule(VkDevice vkDevice, const asset::Shader& asset) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize                 = asset.getSize();
  createInfo.pCode                    = reinterpret_cast<const uint32_t*>(asset.getData());
  VkShaderModule result;
  checkVkResult(vkCreateShaderModule(vkDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

Shader::Shader(log::Logger* logger, const Device* device, const asset::Shader* asset) :
    m_logger{logger}, m_device{device} {
  assert(device);
  assert(asset);

  m_vkModule = createShaderModule(m_device->getVkDevice(), *asset);

  LOG_D(m_logger, "Shader created", {"asset", asset->getId()});
}

Shader::~Shader() { vkDestroyShaderModule(m_device->getVkDevice(), m_vkModule, nullptr); }

} // namespace tria::gfx::internal

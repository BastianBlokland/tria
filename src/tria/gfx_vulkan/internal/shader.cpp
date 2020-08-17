#include "shader.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "tria/gfx/err/gfx_err.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createShaderModule(VkDevice vkDevice, const asset::Shader& asset) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize                 = asset.getSize();
  createInfo.pCode                    = reinterpret_cast<const uint32_t*>(asset.getBegin());
  VkShaderModule result;
  checkVkResult(vkCreateShaderModule(vkDevice, &createInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto getVkStageForShaderKind(asset::ShaderKind kind) {
  switch (kind) {
  case asset::ShaderKind::SpvVertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case asset::ShaderKind::SpvFragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  throw err::GfxErr{"Unsupported shader-kind"};
}

} // namespace

Shader::Shader(log::Logger* logger, const Device* device, const asset::Shader* asset) :
    m_logger{logger}, m_device{device}, m_entryPointName{asset->getEntryPointName()} {
  assert(device);
  assert(asset);

  m_vkStage  = getVkStageForShaderKind(asset->getShaderKind());
  m_vkModule = createShaderModule(m_device->getVkDevice(), *asset);
  DBG_SHADER_NAME(m_device, m_vkModule, asset->getId());

  LOG_D(m_logger, "Vulkan shader module created", {"asset", asset->getId()});
}

Shader::~Shader() { vkDestroyShaderModule(m_device->getVkDevice(), m_vkModule, nullptr); }

} // namespace tria::gfx::internal

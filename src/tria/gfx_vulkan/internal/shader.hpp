#pragma once
#include "tria/asset/shader.hpp"
#include "tria/log/api.hpp"
#include <string_view>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/* Shader resource.
 */
class Shader final {
public:
  using AssetType = asset::Shader;

  Shader(log::Logger* logger, const Device* device, const asset::Shader* asset);
  Shader(const Shader& rhs) = delete;
  Shader(Shader&& rhs)      = delete;
  ~Shader();

  auto operator=(const Shader& rhs) -> Shader& = delete;
  auto operator=(Shader&& rhs) -> Shader& = delete;

  [[nodiscard]] auto getVkStage() const noexcept { return m_vkStage; }
  [[nodiscard]] auto getVkModule() const noexcept { return m_vkModule; }
  [[nodiscard]] auto getEntryPointName() const noexcept -> std::string_view {
    return m_entryPointName;
  }

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkShaderStageFlagBits m_vkStage;
  VkShaderModule m_vkModule;
  std::string m_entryPointName;
};

} // namespace tria::gfx::internal

#pragma once
#include "tria/asset/shader.hpp"
#include "tria/log/api.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

class Shader final {
public:
  Shader(log::Logger* logger, const Device* device, const asset::Shader* asset);
  Shader(const Shader& rhs) = delete;
  Shader(Shader&& rhs)      = delete;
  ~Shader();

  auto operator=(const Shader& rhs) -> Shader& = delete;
  auto operator=(Shader&& rhs) -> Shader& = delete;

  [[nodiscard]] auto getVkModule() const noexcept { return m_vkModule; }

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkShaderModule m_vkModule;
};

} // namespace tria::gfx::internal

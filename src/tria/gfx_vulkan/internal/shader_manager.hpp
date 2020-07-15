#pragma once
#include "shader.hpp"
#include "tria/log/api.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>

namespace tria::gfx::internal {

class Device;

class ShaderManager final {
public:
  ShaderManager(log::Logger* logger, const Device* device) : m_logger{logger}, m_device{device} {
    assert(m_device);
  }
  ~ShaderManager() = default;

  [[nodiscard]] auto getShader(const asset::Shader* asset) noexcept -> const Shader&;

private:
  log::Logger* m_logger;
  const Device* m_device;
  std::unordered_map<const asset::Shader*, Shader> m_data;
};

using ShaderManagerUnique = std::unique_ptr<ShaderManager>;

} // namespace tria::gfx::internal

#include "shader_manager.hpp"

namespace tria::gfx::internal {

auto ShaderManager::getShader(const asset::Shader* asset) noexcept -> const Shader& {

  // If we already have a shader for the given asset then return that.
  const auto itr = m_data.find(asset);
  if (itr != m_data.end()) {
    return itr->second;
  }

  // Otherwise construct a new shader.
  const auto insertItr = m_data.try_emplace(asset, m_logger, m_device, asset);
  return insertItr.first->second;
}

} // namespace tria::gfx::internal

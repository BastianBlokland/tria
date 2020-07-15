#include "graphic_manager.hpp"

namespace tria::gfx::internal {

auto GraphicManager::getGraphic(const asset::Graphic* asset, VkRenderPass vkRenderPass) noexcept
    -> const Graphic& {

  // If we already have a graphic for the given asset then return that.
  const auto itr = m_data.find(asset);
  if (itr != m_data.end()) {
    return itr->second;
  }

  // Otherwise construct a new graphic.
  const auto insertItr =
      m_data.try_emplace(asset, m_logger, m_device, m_shaderManager, vkRenderPass, asset);
  return insertItr.first->second;
}

} // namespace tria::gfx::internal

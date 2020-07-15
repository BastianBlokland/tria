#include "graphic_manager.hpp"

namespace tria::gfx::internal {

auto GraphicManager::getGraphic(const asset::Graphic* asset, VkRenderPass vkRenderPass) noexcept
    -> const Graphic& {

  // If we already have a graphic for the given asset then return that.
  const auto itr = m_data.find(asset);
  if (itr != m_data.end()) {
    return *itr->second;
  }

  // Otherwise construct a new graphic.
  auto newGraphic      = std::make_unique<Graphic>(m_logger, m_device, vkRenderPass, asset);
  const auto insertItr = m_data.insert({asset, std::move(newGraphic)});
  return *insertItr.first->second;
}

} // namespace tria::gfx::internal

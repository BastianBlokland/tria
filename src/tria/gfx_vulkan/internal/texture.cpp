#include "texture.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Texture::Texture(log::Logger* logger, Device* device, const asset::Texture* asset) :
    m_asset{asset} {

  assert(device);
  assert(m_asset);

  LOG_D(logger, "Vulkan texture created", {"asset", asset->getId()});
}

auto Texture::prepareResources(Transferer* /*unused*/) const -> void {}

} // namespace tria::gfx::internal

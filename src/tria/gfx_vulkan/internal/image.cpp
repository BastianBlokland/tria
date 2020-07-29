#include "image.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Image::Image(log::Logger* logger, Device* device, const asset::Image* asset) : m_asset{asset} {

  assert(device);
  assert(m_asset);

  LOG_D(logger, "Vulkan image created", {"asset", asset->getId()});
}

auto Image::prepareResources(Transferer* /*unused*/) const -> void {}

} // namespace tria::gfx::internal

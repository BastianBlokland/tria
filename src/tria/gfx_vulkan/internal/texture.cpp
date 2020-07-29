#include "texture.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

Texture::Texture(log::Logger* logger, Device* device, const asset::Texture* asset) :
    m_asset{asset}, m_imageUploaded{false} {

  assert(device);
  assert(m_asset);

  const auto vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
  assert(getVkFormatSize(vkFormat) == sizeof(asset::Pixel));
  assert(getVkFormatChannelCount(vkFormat) == 4U);
  m_image = Image{device, m_asset->getSize(), vkFormat};

  LOG_D(
      logger,
      "Vulkan texture created",
      {"asset", asset->getId()},
      {"size", asset->getSize()},
      {"format", getVkFormatString(vkFormat)},
      {"memory", log::MemSize{m_image.getMemSize()}});
}

auto Texture::prepareResources(Transferer* transferer) const -> void {
  if (!m_imageUploaded) {

    // Upload pixels to the image.
    transferer->queueTransfer(m_asset->getPixelBegin(), m_image);

    m_imageUploaded = true;
  }
}

} // namespace tria::gfx::internal

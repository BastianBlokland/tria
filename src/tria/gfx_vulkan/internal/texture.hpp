#pragma once
#include "image.hpp"
#include "transferer.hpp"
#include "tria/asset/texture.hpp"
#include "tria/log/api.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Texture resource.
 * Holds reference to pixel data on the gpu.
 */
class Texture final {
public:
  using AssetType = asset::Texture;

  Texture(log::Logger* logger, Device* device, const asset::Texture* asset);
  Texture(const Texture& rhs) = delete;
  Texture(Texture&& rhs)      = delete;
  ~Texture()                  = default;

  auto operator=(const Texture& rhs) -> Texture& = delete;
  auto operator=(Texture&& rhs) -> Texture& = delete;

  [[nodiscard]] auto getImage() const noexcept -> const Image& { return m_image; }

  /* Note: Call this before accessing any resources from this texture.
   */
  auto prepareResources(Transferer* transferer) const -> void;

private:
  const asset::Texture* m_asset;
  mutable bool m_imageUploaded;
  Image m_image;
};

} // namespace tria::gfx::internal

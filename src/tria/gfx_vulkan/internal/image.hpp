#pragma once
#include "buffer.hpp"
#include "transferer.hpp"
#include "tria/asset/image.hpp"
#include "tria/log/api.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Image resource.
 * Holds reference to pixel data on the gpu.
 */
class Image final {
public:
  using AssetType = asset::Image;

  Image(log::Logger* logger, Device* device, const asset::Image* asset);
  Image(const Image& rhs) = delete;
  Image(Image&& rhs)      = delete;
  ~Image()                = default;

  auto operator=(const Image& rhs) -> Image& = delete;
  auto operator=(Image&& rhs) -> Image& = delete;

  /* Note: Call this before accessing any resources from this image.
   */
  auto prepareResources(Transferer* transferer) const -> void;

private:
  const asset::Image* m_asset;
};

} // namespace tria::gfx::internal

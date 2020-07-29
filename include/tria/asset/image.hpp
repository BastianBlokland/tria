#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/vec.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

namespace tria::asset {

using ImageSize = math::Vec<uint16_t, 2>;
using Pixel     = math::Vec<uint8_t, 4>;

/*
 * Asset containing pixel data.
 * Each pixel is 32 bit, R, G, B, A with 8 bit per component.
 */
class Image final : public Asset {
public:
  Image(AssetId id, ImageSize size, std::vector<Pixel> pixels) :
      Asset{std::move(id), getKind()}, m_size{size}, m_pixels{std::move(pixels)} {
    assert(m_pixels.size() == static_cast<size_t>(size.x() * size.y()));
    assert(m_pixels.size() > 0);
  }
  Image(const Image& rhs) = delete;
  Image(Image&& rhs)      = delete;
  ~Image() noexcept       = default;

  auto operator=(const Image& rhs) -> Image& = delete;
  auto operator=(Image&& rhs) -> Image& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Image; }

  [[nodiscard]] auto getSize() const noexcept { return m_size; }

  [[nodiscard]] auto getPixelCount() const noexcept { return m_pixels.size(); }
  [[nodiscard]] auto getPixelBegin() const noexcept -> const Pixel* { return m_pixels.data(); }
  [[nodiscard]] auto getPixelEnd() const noexcept -> const Pixel* {
    return m_pixels.data() + m_pixels.size();
  }

private:
  ImageSize m_size;
  std::vector<Pixel> m_pixels;
};

} // namespace tria::asset

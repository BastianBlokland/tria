#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/vec.hpp"
#include <cassert>
#include <cstdint>

namespace tria::asset {

using TextureSize = math::Vec<uint16_t, 2>;
using Pixel       = math::Vec<uint8_t, 4>;

/*
 * Asset containing pixel data.
 * Each pixel is 32 bit, R, G, B, A with 8 bit per component.
 */
class Texture final : public Asset {
public:
  Texture(AssetId id, TextureSize size, math::PodVector<Pixel> pixels) :
      Asset{std::move(id), getKind()}, m_size{size}, m_pixels{std::move(pixels)} {
    assert(m_pixels.size() == static_cast<size_t>(size.x() * size.y()));
    assert(m_pixels.size() > 0);
  }
  Texture(const Texture& rhs) = delete;
  Texture(Texture&& rhs)      = delete;
  ~Texture() noexcept         = default;

  auto operator=(const Texture& rhs) -> Texture& = delete;
  auto operator=(Texture&& rhs) -> Texture& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Texture; }

  /* Get the size in pixels of the texture.
   */
  [[nodiscard]] auto getSize() const noexcept { return m_size; }

  /* Get the aspect ratio of the texture (width / height).
   */
  [[nodiscard]] auto getAspect() const noexcept -> float {
    return static_cast<float>(m_size.x()) / static_cast<float>(m_size.y());
  }

  [[nodiscard]] auto getPixelCount() const noexcept { return m_pixels.size(); }
  [[nodiscard]] auto getPixelBegin() const noexcept { return m_pixels.begin(); }
  [[nodiscard]] auto getPixelEnd() const noexcept { return m_pixels.end(); }

private:
  TextureSize m_size;
  math::PodVector<Pixel> m_pixels;
};

} // namespace tria::asset

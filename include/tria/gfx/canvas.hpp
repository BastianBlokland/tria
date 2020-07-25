#pragma once
#include "tria/asset/graphic.hpp"
#include "tria/math/vec.hpp"
#include <cstdint>
#include <memory>
#include <type_traits>

namespace tria::gfx {

class Context;
class NativeCanvas;

/*
 * Abstraction over a Canvas that can be rendered to.
 *
 * Note: Api is NOT thread-safe.
 */
class Canvas final {
  friend Context;

public:
  Canvas(const Canvas& rhs)     = delete;
  Canvas(Canvas&& rhs) noexcept = default;
  ~Canvas();

  auto operator=(const Canvas& rhs) -> Canvas& = delete;
  auto operator=(Canvas&& rhs) noexcept -> Canvas& = default;

  /* Begin drawing.
   * Note: Has to be followed with a call to 'drawEnd' before calling 'drawBegin' again.
   * Returns: false if we failed to begin drawing (for example because the window is minimized).
   */
  [[nodiscard]] auto drawBegin(math::Color clearCol) -> bool;

  /* Draw a single instance of the given graphic.
   */
  template <typename UniformDataType>
  auto draw(const asset::Graphic* asset, const UniformDataType& uniData) -> void {
    static_assert(
        std::is_trivially_copyable<UniformDataType>::value,
        "Uniform data type has to be trivially copyable");

    draw(asset, &uniData, sizeof(UniformDataType));
  }

  /* Draw a single instance of the given graphic.
   */
  auto draw(const asset::Graphic* asset, const void* uniData, size_t uniSize) -> void;

  /* End drawing and present the result to the window.
   * Note: Has to be preceeded by a call to 'drawBegin'
   */
  auto drawEnd() -> void;

private:
  std::unique_ptr<NativeCanvas> m_native;

  Canvas(std::unique_ptr<NativeCanvas> native);
};

} // namespace tria::gfx

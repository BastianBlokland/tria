#pragma once
#include "tria/asset/graphic.hpp"
#include <memory>

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
  [[nodiscard]] auto drawBegin() -> bool;

  /* Draw a single instance of the given graphic.
   */
  auto draw(const asset::Graphic* asset) -> void;

  /* End drawing and present the result to the window.
   * Note: Has to be preceeded by a call to 'drawBegin'
   */
  auto drawEnd() -> void;

private:
  std::unique_ptr<NativeCanvas> m_native;

  Canvas(std::unique_ptr<NativeCanvas> native);
};

} // namespace tria::gfx

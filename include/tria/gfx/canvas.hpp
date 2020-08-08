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
        std::is_trivially_copyable_v<UniformDataType>,
        "Uniform data type has to be trivially copyable");
    static_assert(
        std::alignment_of_v<UniformDataType> == 16,
        "Uniform data type has to be aligned to 16 bytes");

    draw(asset, &uniData, sizeof(UniformDataType), 1U);
  }

  /* Draw multple instances of the given graphic.
   */
  template <typename UniformDataType>
  auto draw(const asset::Graphic* asset, UniformDataType* uniDataBegin, UniformDataType* uniDataEnd)
      -> void {
    static_assert(
        std::is_trivially_copyable_v<UniformDataType>,
        "Uniform data type has to be trivially copyable");
    static_assert(
        std::alignment_of_v<UniformDataType> == 16,
        "Uniform data type has to be aligned to 16 bytes");

    assert(uniDataBegin <= uniDataEnd);

    const auto count = static_cast<uint32_t>(uniDataEnd - uniDataBegin);
    draw(asset, uniDataBegin, sizeof(UniformDataType), count);
  }

  /* Draw 'count' instances of the given graphic.
   * Note: Make sure that 'count' * 'uniSize' of data is available at the 'uniData' pointer.
   */
  auto draw(const asset::Graphic* asset, const void* uniData, size_t uniSize, uint32_t count)
      -> void;

  /* End drawing and present the result to the window.
   * Note: Has to be preceeded by a call to 'drawBegin'
   */
  auto drawEnd() -> void;

private:
  std::unique_ptr<NativeCanvas> m_native;

  Canvas(std::unique_ptr<NativeCanvas> native);
};

} // namespace tria::gfx

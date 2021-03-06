#pragma once
#include "tria/asset/graphic.hpp"
#include "tria/math/vec.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace tria::gfx {

/* Statistics for a frame that has been completed.
 */
struct DrawStats {
  std::chrono::duration<double> gpuTime;
  uint64_t inputAssemblyVerts;
  uint64_t inputAssemblyPrimitives;
  uint64_t vertShaderInvocations;
  uint64_t fragShaderInvocations;
};

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

  /* Get statistics for the last draw.
   * Note: Blocks if the previous draw has not finished executing.
   */
  [[nodiscard]] auto getDrawStats() const noexcept -> DrawStats;

  /* Begin drawing.
   * Note: Has to be followed with a call to 'drawEnd' before calling 'drawBegin' again.
   * Returns: false if we failed to begin drawing (for example because the window is minimized).
   */
  [[nodiscard]] auto drawBegin(math::Color clearCol = math::color::soothingPurple()) -> bool;

  /* Bind global data to be used in draws.
   * Note: Only a single global data binding is active, and does not persist after 'drawEnd'.
   */
  template <typename GlobalDataType>
  auto bindGlobalData(const GlobalDataType& instData) -> void {
    static_assert(
        std::is_trivially_copyable_v<GlobalDataType>,
        "Global data type has to be trivially copyable");
    bindGlobalData(&instData, sizeof(GlobalDataType));
  }

  /* Bind global data to be used in draws.
   * Note: Only a single global data binding is active, and does not persist after 'drawEnd'.
   */
  auto bindGlobalData(const void* data, size_t dataSize) -> void;

  /* Draw a single instance of a graphic without instance data.
   */
  auto draw(const asset::Graphic* asset) -> void { draw(asset, 0U, nullptr, 0U, 1U); }

  /* Draw a single instance of a graphic without instance data.
   */
  auto draw(const asset::Graphic* asset, uint32_t indexCount) -> void {
    draw(asset, indexCount, nullptr, 0U, 1U);
  }

  /* Draw a single instance of a graphic with instance data.
   */
  template <typename InstDataType>
  auto draw(const asset::Graphic* asset, const InstDataType& instData) -> void {
    return draw(asset, 0U, instData);
  }

  /* Draw a single instance of a graphic with instance data.
   */
  template <typename InstDataType>
  auto draw(const asset::Graphic* asset, uint32_t indexCount, const InstDataType& instData)
      -> void {
    static_assert(
        std::is_trivially_copyable_v<InstDataType>,
        "Instance data type has to be trivially copyable");
    draw(asset, indexCount, &instData, sizeof(InstDataType), 1U);
  }

  /* Draw multple instances of a graphic with instance data.
   */
  template <typename InstDataType>
  auto draw(const asset::Graphic* asset, InstDataType* instDataBegin, InstDataType* instDataEnd)
      -> void {
    draw<InstDataType>(asset, 0U, instDataBegin, instDataEnd);
  }

  /* Draw multple instances of a graphic with instance data.
   */
  template <typename InstDataType>
  auto draw(
      const asset::Graphic* asset,
      uint32_t indexCount,
      InstDataType* instDataBegin,
      InstDataType* instDataEnd) -> void {
    static_assert(
        std::is_trivially_copyable_v<InstDataType>,
        "Instance data type has to be trivially copyable");
    static_assert(
        std::alignment_of_v<InstDataType> == 16,
        "Instance data type has to be aligned to 16 bytes");

    assert(instDataBegin <= instDataEnd);

    const auto count = static_cast<uint32_t>(instDataEnd - instDataBegin);
    draw(asset, indexCount, instDataBegin, sizeof(InstDataType), count);
  }

  /* Draw 'count' instances of the given graphic.
   * Note: IndexCount of 0 will draw all indices in the mesh of the graphic.
   * Note: Make sure that 'count' * 'instDataSize' of data is available at the 'instData' pointer.
   */
  auto draw(
      const asset::Graphic* asset,
      uint32_t indexCount,
      const void* instData,
      size_t instDataSize,
      uint32_t count) -> void;

  /* End drawing and present the result to the window.
   * Note: Has to be preceeded by a call to 'drawBegin'
   */
  auto drawEnd() -> void;

private:
  std::unique_ptr<NativeCanvas> m_native;

  Canvas(std::unique_ptr<NativeCanvas> native);
};

} // namespace tria::gfx

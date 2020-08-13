#pragma once
#include "tria/gfx/canvas.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"

namespace tria::gfx {

class NativeContext;

enum class VSyncMode {
  Disable,
  Enable,
};

enum class DepthMode {
  Enable,  // Depth testing active.
  Disable, // No depth testing.
};

using ClearMask = uint8_t;

enum class Clear : uint8_t {
  Color = 1U << 0U,
  Depth = 1U << 1U,
};

/*
 * Abstraction over a graphics context.
 *
 * Note: Api is NOT thread-safe.
 */
class Context final {
public:
  Context(log::Logger* logger);
  Context(const Context& rhs)     = delete;
  Context(Context&& rhs) noexcept = default;
  ~Context();

  auto operator=(const Context& rhs) -> Context& = delete;
  auto operator=(Context&& rhs) noexcept -> Context& = default;

  /* Create a canvas to render into that outputs to the given window.
   */
  [[nodiscard]] auto
  createCanvas(const pal::Window* window, VSyncMode vSync, DepthMode depth, ClearMask clear)
      -> Canvas;

private:
  std::unique_ptr<NativeContext> m_native;
};

[[nodiscard]] constexpr auto getName(VSyncMode mode) noexcept -> std::string_view {
  switch (mode) {
  case VSyncMode::Enable:
    return "enable";
  case VSyncMode::Disable:
    return "disable";
  }
  return "unknown";
}

[[nodiscard]] constexpr auto getName(DepthMode mode) noexcept -> std::string_view {
  switch (mode) {
  case DepthMode::Enable:
    return "enable";
  case DepthMode::Disable:
    return "disable";
  }
  return "unknown";
}

[[nodiscard]] constexpr auto noneClearMask() noexcept -> ClearMask { return 0U; }

[[nodiscard]] constexpr auto clearMask(Clear clear) noexcept -> ClearMask {
  return static_cast<ClearMask>(clear);
}

[[nodiscard]] constexpr auto operator|(Clear lhs, Clear rhs) noexcept -> ClearMask {
  return clearMask(lhs) | clearMask(rhs);
}

[[nodiscard]] constexpr auto operator|(ClearMask lhs, Clear rhs) noexcept -> ClearMask {
  return lhs | clearMask(rhs);
}

} // namespace tria::gfx

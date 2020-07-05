#pragma once
#include <memory>

namespace tria::gfx {

class Context;
class NativeSurface;

/*
 * Abstraction over a surface that can be rendered to.
 *
 * Note: Api is NOT thread-safe.
 */
class Surface final {
  friend Context;

public:
  Surface(const Surface& rhs)     = delete;
  Surface(Surface&& rhs) noexcept = default;
  ~Surface();

  auto operator=(const Surface& rhs) -> Surface& = delete;
  auto operator=(Surface&& rhs) noexcept -> Surface& = default;

private:
  std::unique_ptr<NativeSurface> m_native;

  Surface(std::unique_ptr<NativeSurface> native);
};

} // namespace tria::gfx

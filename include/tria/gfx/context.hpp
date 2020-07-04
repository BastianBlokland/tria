#pragma once
#include "tria/log/api.hpp"

namespace tria::gfx {

class NativeContext;

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

private:
  std::unique_ptr<NativeContext> m_native;
};

} // namespace tria::gfx

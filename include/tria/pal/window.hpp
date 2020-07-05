#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>

namespace tria::pal {

class NativePlatform;

/* Handle to a native window.
 * When handle is destroyed the native window is also closed. Supports moving ownership.
 *
 * Note: Api is not thread-safe.
 */
class Window final {
  friend NativePlatform;

public:
  Window()                  = delete;
  Window(const Window& rhs) = delete;
  Window(Window&& rhs) noexcept {
    assert(rhs.m_alive);
    m_platform  = rhs.m_platform;
    m_id        = rhs.m_id;
    m_alive     = true;
    rhs.m_alive = false;
  }
  ~Window();

  auto operator=(const Window& rhs) -> Window& = delete;

  auto operator=(Window&& rhs) noexcept -> Window& {
    assert(rhs.m_alive);
    m_platform  = rhs.m_platform;
    m_id        = rhs.m_id;
    m_alive     = true;
    rhs.m_alive = false;
    return *this;
  }

  /* Has the user requested to close this window.
   */
  [[nodiscard]] auto getIsCloseRequested() const noexcept -> bool;

  [[nodiscard]] auto getWidth() const noexcept -> uint16_t;
  [[nodiscard]] auto getHeight() const noexcept -> uint16_t;

  auto setTitle(std::string_view title) -> void;
  auto setSize(uint16_t width, uint16_t height) -> void;

  [[nodiscard]] auto getNativePlatformPtr() const noexcept { return m_platform; }
  [[nodiscard]] auto getWindowId() const noexcept { return m_id; }

private:
  using WindowId = uint32_t;

  NativePlatform* m_platform;
  WindowId m_id;
  bool m_alive;

  Window(NativePlatform* platform, WindowId id) noexcept :
      m_platform{platform}, m_id{id}, m_alive{true} {
    assert(m_platform);
  }
};

} // namespace tria::pal

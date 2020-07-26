#pragma once
#include "tria/math/vec.hpp"
#include "tria/pal/key.hpp"
#include <cassert>
#include <cstdint>
#include <string_view>

namespace tria::pal {

class NativePlatform;

using WindowSize   = math::Vec<uint16_t, 2>;
using WindowPos    = math::Vec<int32_t, 2>;
using WindowPosNrm = math::Vec<float, 2>;

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

  /* Get the size of the window in pixels.
   */
  [[nodiscard]] auto getSize() const noexcept -> WindowSize;

  /* Has the user requested to close this window.
   */
  [[nodiscard]] auto getIsCloseRequested() const noexcept -> bool;

  /* Get the mouse position in pixels.
   * Upper left is {0, 0} and lower right is {getSize().x(), getSize().y()}.
   */
  [[nodiscard]] auto getMousePos() const noexcept -> WindowPos;

  /* Get the normalized mouse position.
   * Upper left is {0.0, 0.0} and lower right is {1.0, 1.0}.
   * Note: when window size is {0, 0} this will return {nan, nan}.
   */
  [[nodiscard]] auto getMousePosNrm() const noexcept -> WindowPosNrm {
    return WindowPosNrm{getMousePos()} / WindowPosNrm{getSize()};
  }

  /* Check if the given key is currently being held down.
   */
  [[nodiscard]] auto isKeyDown(Key key) const noexcept -> bool;

  auto setTitle(std::string_view title) -> void;
  auto setSize(WindowSize size) -> void;

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

#pragma once
#include "tria/pal/key.hpp"
#include "tria/pal/window.hpp"

namespace tria::pal::internal {

class WindowInput final {
public:
  WindowInput() = default;

  [[nodiscard]] auto getMousePos() const noexcept { return m_mousePos; }
  [[nodiscard]] auto getIsCloseRequested() const noexcept { return m_isCloseRequested; }
  [[nodiscard]] auto isKeyDown(Key key) const noexcept { return isInMask(m_downKeys, key); }
  [[nodiscard]] auto isKeyPressed(Key key) const noexcept { return isInMask(m_pressedKeys, key); }
  [[nodiscard]] auto isKeyReleased(Key key) const noexcept { return isInMask(m_releasedKeys, key); }

  /* Reset temporary state like the 'pressed' and 'released' keys. Usually called before handing new
   * platform events.
   */
  auto reset() {
    m_pressedKeys  = KeyMask();
    m_releasedKeys = KeyMask();
  }

  auto setMousePos(WindowPos pos) { m_mousePos = pos; }

  auto requestClose() { m_isCloseRequested = true; }

  auto markPressed(Key key) {
    if (!isInMask(m_downKeys, key)) {
      m_downKeys |= KeyMask(key);
      m_pressedKeys |= KeyMask(key);
    }
  }

  auto markReleased(Key key) {
    if (isInMask(m_downKeys, key)) {
      m_downKeys &= ~KeyMask(key);
      m_releasedKeys |= KeyMask(key);
    }
  }

private:
  WindowPos m_mousePos;
  KeyMask m_downKeys;
  KeyMask m_pressedKeys;
  KeyMask m_releasedKeys;
  bool m_isCloseRequested;
};

} // namespace tria::pal::internal

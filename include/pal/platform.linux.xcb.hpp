#pragma once
#include "window.linux.xcb.hpp"
#include <list>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

namespace pal {

class Platform final {
public:
  using WindowIterator = typename std::list<Window>::iterator;

  Platform()                    = default;
  Platform(const Platform& rhs) = delete;
  Platform(Platform&& rhs)      = default;
  ~Platform();

  auto operator=(const Platform& rhs) -> Platform& = delete;
  auto operator=(Platform&& rhs) noexcept -> Platform& = default;

  [[nodiscard]] auto beginWindows() -> WindowIterator { return m_windows.begin(); };
  [[nodiscard]] auto endWindows() -> WindowIterator { return m_windows.end(); };

  auto handleEvents() -> void;

  auto createWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height) -> Window&;
  auto createWindow(uint16_t width, uint16_t height) -> Window& {
    return createWindow(0, 0, width, height);
  }
  auto destroyWindow(const Window& win) -> void;

private:
  xcb_connection_t* m_xcbCon;
  xcb_screen_t* m_xcbScreen;
  xcb_atom_t m_xcbProtoMsgAtom;
  xcb_atom_t m_xcbDeleteMsgAtom;
  std::list<Window> m_windows;

  auto xcbSetup() -> void;
  auto xcbTeardown() noexcept -> void;
  auto xcbCheckErr() -> void;
  auto xcbGetAtom(const std::string& name) noexcept -> xcb_atom_t;
  auto getWindow(xcb_window_t xcbId) noexcept -> Window*;
};

} // namespace pal

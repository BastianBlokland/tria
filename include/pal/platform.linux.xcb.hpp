#pragma once
#include "window.linux.xcb.hpp"
#include <cstdint>
#include <list>
#include <string>
#include <string_view>
#include <xcb/xcb.h>

namespace pal {

class Platform final {
public:
  using WindowIterator = typename std::list<Window>::iterator;

  explicit Platform(std::string appName) :
      m_appName{std::move(appName)}, m_xcbCon{nullptr}, m_xcbScreen{nullptr} {}
  Platform(const Platform& rhs)     = delete;
  Platform(Platform&& rhs) noexcept = delete;
  ~Platform();

  auto operator=(const Platform& rhs) -> Platform& = delete;
  auto operator=(Platform&& rhs) noexcept -> Platform& = delete;

  [[nodiscard]] auto getAppName() const noexcept -> std::string_view { return m_appName; }

  [[nodiscard]] auto beginWindows() noexcept -> WindowIterator { return m_windows.begin(); };
  [[nodiscard]] auto endWindows() noexcept -> WindowIterator { return m_windows.end(); };

  auto handleEvents() -> void;

  auto createWindow(uint16_t width, uint16_t height) -> Window&;
  auto destroyWindow(const Window& win) -> void;

private:
  std::string m_appName;
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

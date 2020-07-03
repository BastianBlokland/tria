#pragma once
#include "tria/log/api.hpp"
#include "tria/pal/err/display_protocol_err.hpp"
#include "tria/pal/platform.hpp"
#include <string>
#include <unordered_map>
#include <xcb/xcb.h>

namespace tria::pal {

using WindowId = uint32_t;

struct WindowData {
  WindowId id;
  uint16_t width;
  uint16_t height;
  bool isCloseRequested;

  WindowData(WindowId id, uint16_t width, uint16_t height) noexcept :
      id{id}, width{width}, height{height}, isCloseRequested{false} {}
};

class NativePlatform final {
public:
  NativePlatform(log::Logger* logger, std::string appName) :
      m_logger{logger}, m_appName{std::move(appName)}, m_xcbCon{nullptr}, m_xcbScreen{nullptr} {}

  ~NativePlatform();

  [[nodiscard]] auto getAppName() const noexcept -> std::string_view { return m_appName; }

  [[nodiscard]] auto getIsWinCloseRequested(WindowId id) const noexcept -> bool {
    auto* win = getWindow(id);
    assert(win);
    return win->isCloseRequested;
  }

  [[nodiscard]] auto getWinWidth(WindowId id) const noexcept -> uint16_t {
    auto* win = getWindow(id);
    assert(win);
    return win->width;
  }

  [[nodiscard]] auto getWinHeight(WindowId id) const noexcept -> uint16_t {
    auto* win = getWindow(id);
    assert(win);
    return win->height;
  }

  auto handleEvents() -> void;

  auto createWindow(uint16_t width, uint16_t height) -> Window;

  auto destroyWindow(WindowId id) noexcept -> void;

  auto setWinTitle(WindowId id, std::string_view title) noexcept -> void;

  auto setWinSize(WindowId id, uint16_t width, uint16_t height) noexcept -> void;

private:
  log::Logger* m_logger;
  std::string m_appName;
  xcb_connection_t* m_xcbCon;
  xcb_screen_t* m_xcbScreen;
  xcb_atom_t m_xcbProtoMsgAtom;
  xcb_atom_t m_xcbDeleteMsgAtom;
  std::unordered_map<WindowId, WindowData> m_windows;

  auto xcbSetup() -> void;

  auto xcbTeardown() noexcept -> void;

  auto xcbCheckErr() -> void;

  auto xcbGetAtom(const std::string& name) noexcept -> xcb_atom_t;

  auto getWindow(WindowId id) noexcept -> WindowData*;

  auto getWindow(WindowId id) const noexcept -> const WindowData*;
};

} // namespace tria::pal

#pragma once
#include "internal/win_input.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/platform.hpp"
#include <string>
#include <unordered_map>
#include <xcb/xcb.h>

namespace tria::pal {

using WindowId = uint32_t;

struct WindowData {
  WindowId id;
  WindowSize size;
  internal::WinInput input;

  WindowData(WindowId id, WindowSize size) noexcept : id{id}, size{size}, input{} {}
};

class NativePlatform final {
public:
  NativePlatform(log::Logger* logger);
  ~NativePlatform();

  [[nodiscard]] auto getConnection() const noexcept { return m_xcbCon; }

  [[nodiscard]] auto getWinSize(WindowId id) const noexcept -> WindowSize {
    auto* win = getWindow(id);
    assert(win);
    return win->size;
  }

  [[nodiscard]] auto getWinInput(WindowId id) const noexcept -> const internal::WinInput& {
    auto* win = getWindow(id);
    assert(win);
    return win->input;
  }

  auto handleEvents() -> void;

  auto createWindow(WindowSize size) -> Window;

  auto destroyWindow(WindowId id) noexcept -> void;

  auto setWinTitle(WindowId id, std::string_view title) noexcept -> void;

  auto setWinSize(WindowId id, WindowSize size) noexcept -> void;

private:
  log::Logger* m_logger;
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

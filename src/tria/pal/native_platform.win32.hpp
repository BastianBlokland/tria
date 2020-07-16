#pragma once
#include "tria/log/api.hpp"
#include "tria/pal/platform.hpp"
#include <string>
#include <unordered_map>
#include <windows.h>

namespace tria::pal {

using WindowId = uint32_t;

constexpr auto windowMinWidth  = 128;
constexpr auto windowMinHeight = 128;

struct WindowData {
  WindowId id;
  HWND handle;
  std::string className;
  DWORD dwStyle;
  WindowSize size;
  bool isCloseRequested;

  WindowData(
      WindowId id, HWND handle, std::string className, DWORD dwStyle, WindowSize size) noexcept :
      id{id},
      handle{handle},
      className{std::move(className)},
      dwStyle{dwStyle},
      size{size},
      isCloseRequested{false} {}
};

auto WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT;

class NativePlatform final {
  friend auto WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT;

public:
  NativePlatform(log::Logger* logger);
  ~NativePlatform();

  [[nodiscard]] auto getHInstance() const noexcept { return m_hInstance; }

  [[nodiscard]] auto getAppName() const noexcept -> std::string_view { return m_appName; }

  [[nodiscard]] auto getIsWinCloseRequested(WindowId id) const noexcept -> bool {
    auto* win = getWindow(id);
    assert(win);
    return win->isCloseRequested;
  }

  [[nodiscard]] auto getWinSize(WindowId id) const noexcept -> WindowSize {
    auto* win = getWindow(id);
    assert(win);
    return win->size;
  }

  [[nodiscard]] auto getHWnd(WindowId id) const noexcept -> HWND { return getWindow(id)->handle; }

  auto handleEvents() -> void;

  auto createWindow(WindowSize size) -> Window;

  auto destroyWindow(WindowId id) noexcept -> void;

  auto setWinTitle(WindowId id, std::string_view title) noexcept -> void;

  auto setWinSize(WindowId id, WindowSize size) noexcept -> void;

private:
  log::Logger* m_logger;
  std::string m_appName;
  HINSTANCE m_hInstance;
  WindowId m_nextWinId;
  std::unordered_map<WindowId, WindowData> m_windows;

  auto win32Setup() -> void;

  auto handleEvent(HWND hWnd, UINT msg, WPARAM /*unused*/, LPARAM lParam) noexcept -> bool;

  auto getWindow(HWND handle) noexcept -> WindowData*;

  auto getWindow(WindowId id) noexcept -> WindowData*;

  auto getWindow(WindowId id) const noexcept -> const WindowData*;
};

} // namespace tria::pal

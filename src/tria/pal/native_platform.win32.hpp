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
  uint16_t width;
  uint16_t height;
  bool isCloseRequested;

  WindowData(
      WindowId id,
      HWND handle,
      std::string className,
      DWORD dwStyle,
      uint16_t width,
      uint16_t height) noexcept :
      id{id},
      handle{handle},
      className{std::move(className)},
      dwStyle{dwStyle},
      width{width},
      height{height},
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

  [[nodiscard]] auto getHWnd(WindowId id) const noexcept -> HWND { return getWindow(id)->handle; }

  auto handleEvents() -> void;

  auto createWindow(uint16_t width, uint16_t height) -> Window;

  auto destroyWindow(WindowId id) noexcept -> void;

  auto setWinTitle(WindowId id, std::string_view title) noexcept -> void;

  auto setWinSize(WindowId id, uint16_t width, uint16_t height) noexcept -> void;

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

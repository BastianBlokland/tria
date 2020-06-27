#pragma once
#include "window.win32.hpp"
#include <list>
#include <string>
#include <string_view>
#include <windows.h>

namespace pal {

class Platform final {
  friend auto WindowProc(HWND, UINT, WPARAM, LPARAM) noexcept -> LRESULT;

public:
  using WindowIterator = typename std::list<Window>::iterator;

  explicit Platform(std::string appName) :
      m_appName{std::move(appName)}, m_hInstance{nullptr}, m_winCreateCounter{0} {}
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
  HINSTANCE m_hInstance;
  unsigned int m_winCreateCounter;
  std::list<Window> m_windows;

  auto win32Setup() -> void;
  auto getWindow(HWND winHandle) noexcept -> Window*;

  auto handleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept -> bool;
};

} // namespace pal

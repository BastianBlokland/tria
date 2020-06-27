#pragma once
#include <cstdint>
#include <string>
#include <windows.h>

namespace pal {

class Platform;

class Window final {
  friend class Platform;

public:
  Window()                      = delete;
  Window(const Window& rhs)     = delete;
  Window(Window&& rhs) noexcept = default;

  auto operator=(const Window& rhs) -> Window& = delete;
  auto operator=(Window&& rhs) noexcept -> Window& = delete;

  [[nodiscard]] auto getIsCloseRequested() const noexcept { return m_isCloseRequested; }
  [[nodiscard]] auto getWidth() const noexcept { return m_width; }
  [[nodiscard]] auto getHeight() const noexcept { return m_height; }

  auto setTitle(const std::string& title) -> void;
  auto setSize(uint16_t width, uint16_t height) -> void;

private:
  HWND m_winHandle;
  std::string m_winClassName;
  DWORD m_winDwStyle;
  uint16_t m_width;
  uint16_t m_height;
  bool m_isCloseRequested;

  Window(
      HWND winHandle,
      std::string winClassName,
      DWORD winDwStyle,
      uint16_t width,
      uint16_t height) noexcept :
      m_winHandle{winHandle},
      m_winClassName{std::move(winClassName)},
      m_winDwStyle{winDwStyle},
      m_width{width},
      m_height{height},
      m_isCloseRequested{false} {}
};

} // namespace pal

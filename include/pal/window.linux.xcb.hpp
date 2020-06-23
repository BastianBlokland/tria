#pragma once
#include <memory>
#include <xcb/xcb.h>

namespace pal {

class Platform;

class Window final {
  friend class Platform;

public:
  Window()                  = delete;
  Window(const Window& rhs) = delete;
  Window(Window&& rhs)      = default;

  auto operator=(const Window& rhs) -> Window& = delete;
  auto operator=(Window&& rhs) noexcept -> Window& = default;

  [[nodiscard]] auto getIsCloseRequested() const noexcept { return m_isCloseRequested; }
  [[nodiscard]] auto getX() const noexcept { return m_x; }
  [[nodiscard]] auto getY() const noexcept { return m_y; }
  [[nodiscard]] auto getWidth() const noexcept { return m_width; }
  [[nodiscard]] auto getHeight() const noexcept { return m_height; }

  auto setTitle(const std::string& title) -> void;
  auto setPos(uint16_t x, uint16_t y) -> void;
  auto setSize(uint16_t width, uint16_t height) -> void;

private:
  xcb_connection_t* m_xcbCon;
  xcb_window_t m_xcbWin;
  uint16_t m_x;
  uint16_t m_y;
  uint16_t m_width;
  uint16_t m_height;
  bool m_isCloseRequested;

  Window(
      xcb_connection_t* xcbCon,
      xcb_window_t xcbWin,
      uint16_t x,
      uint16_t y,
      uint16_t width,
      uint16_t height) noexcept :
      m_xcbCon{xcbCon},
      m_xcbWin{xcbWin},
      m_x{x},
      m_y{y},
      m_width{width},
      m_height{height},
      m_isCloseRequested{false} {}
};

} // namespace pal
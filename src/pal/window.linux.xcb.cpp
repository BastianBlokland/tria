#include "pal/window.linux.xcb.hpp"
#include <array>

namespace pal {

auto Window::setTitle(std::string_view title) -> void {

  xcb_change_property(
      m_xcbCon,
      XCB_PROP_MODE_REPLACE,
      m_xcbWin,
      XCB_ATOM_WM_NAME,
      XCB_ATOM_STRING,
      8,
      title.size(),
      title.data());

  xcb_flush(m_xcbCon);
}

auto Window::setSize(uint16_t width, uint16_t height) -> void {

  const auto valList = std::array<uint32_t, 2>{width, height};
  xcb_configure_window(
      m_xcbCon, m_xcbWin, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valList.data());

  xcb_flush(m_xcbCon);

  // Update local information immediately.
  m_width  = width;
  m_height = height;
}

} // namespace pal

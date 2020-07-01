#include "tria/pal/window.win32.hpp"

namespace tria::pal {

auto Window::setTitle(std::string_view title) -> void { SetWindowText(m_winHandle, title.data()); }

auto Window::setSize(uint16_t width, uint16_t height) -> void {

  // Calculate the size of the window (because we give width and height of the content area).
  RECT winRect   = {};
  winRect.right  = static_cast<long>(width);
  winRect.bottom = static_cast<long>(height);
  AdjustWindowRect(&winRect, m_winDwStyle, false);

  // Set the window size.
  SetWindowPos(
      m_winHandle,
      nullptr,
      0,
      0,
      winRect.right - winRect.left,
      winRect.bottom - winRect.top,
      SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
}

} // namespace tria::pal

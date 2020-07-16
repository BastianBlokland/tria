#include "native_platform.xcb.hpp"
#include "tria/pal/window.hpp"
#include <cassert>

namespace tria::pal {

Window::~Window() {
  if (m_alive) {
    m_platform->destroyWindow(m_id);
  }
}

auto Window::getIsCloseRequested() const noexcept -> bool {
  assert(m_alive);
  return m_platform->getIsWinCloseRequested(m_id);
}

auto Window::getSize() const noexcept -> WindowSize {
  assert(m_alive);
  return m_platform->getWinSize(m_id);
}

auto Window::setTitle(std::string_view title) -> void {
  assert(m_alive);
  m_platform->setWinTitle(m_id, title);
}

auto Window::setSize(WindowSize size) -> void {
  assert(m_alive);
  m_platform->setWinSize(m_id, size);
}

} // namespace tria::pal

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

auto Window::getWidth() const noexcept -> uint16_t {
  assert(m_alive);
  return m_platform->getWinWidth(m_id);
}

auto Window::getHeight() const noexcept -> uint16_t {
  assert(m_alive);
  return m_platform->getWinHeight(m_id);
}

auto Window::setTitle(std::string_view title) -> void {
  assert(m_alive);
  m_platform->setWinTitle(m_id, title);
}

auto Window::setSize(uint16_t width, uint16_t height) -> void {
  assert(m_alive);
  m_platform->setWinSize(m_id, width, height);
}

} // namespace tria::pal

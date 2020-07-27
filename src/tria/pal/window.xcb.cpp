#include "native_platform.xcb.hpp"
#include "tria/pal/window.hpp"
#include <cassert>

namespace tria::pal {

Window::~Window() {
  if (m_alive) {
    m_platform->destroyWindow(m_id);
  }
}

auto Window::getSize() const noexcept -> WindowSize {
  assert(m_alive);
  return m_platform->getWinSize(m_id);
}

auto Window::getIsCloseRequested() const noexcept -> bool {
  assert(m_alive);
  return m_platform->getWinInput(m_id).getIsCloseRequested();
}

auto Window::getMousePos() const noexcept -> WindowPos {
  assert(m_alive);
  return m_platform->getWinInput(m_id).getMousePos();
}

auto Window::isKeyDown(Key key) const noexcept -> bool {
  assert(m_alive);
  return m_platform->getWinInput(m_id).isKeyDown(key);
}

auto Window::isKeyPressed(Key key) const noexcept -> bool {
  assert(m_alive);
  return m_platform->getWinInput(m_id).isKeyPressed(key);
}

auto Window::isKeyReleased(Key key) const noexcept -> bool {
  assert(m_alive);
  return m_platform->getWinInput(m_id).isKeyReleased(key);
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

#include "native_platform.xcb.hpp"

namespace tria::pal {

Platform::Platform(log::Logger* logger) : m_native{std::make_unique<NativePlatform>(logger)} {}

Platform::~Platform() = default;

auto Platform::handleEvents() -> void { m_native->handleEvents(); }

auto Platform::createWindow(const WindowSize size) -> Window {
  return m_native->createWindow(size);
}

} // namespace tria::pal

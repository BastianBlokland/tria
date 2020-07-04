#include "native_platform.win32.hpp"

namespace tria::pal {

Platform::Platform(log::Logger* logger) : m_native{std::make_unique<NativePlatform>(logger)} {}

Platform::~Platform() = default;

auto Platform::handleEvents() -> void { m_native->handleEvents(); }

auto Platform::createWindow(uint16_t width, uint16_t height) -> Window {
  return m_native->createWindow(width, height);
}

} // namespace tria::pal

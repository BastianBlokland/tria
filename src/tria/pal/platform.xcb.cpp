#include "native_platform.xcb.hpp"

namespace tria::pal {

Platform::Platform(log::Logger* logger, std::string appName) :
    m_native{std::make_unique<NativePlatform>(logger, std::move(appName))} {}

Platform::~Platform() = default;

auto Platform::getAppName() const noexcept -> std::string_view { return m_native->getAppName(); }

auto Platform::handleEvents() -> void { m_native->handleEvents(); }

auto Platform::createWindow(uint16_t width, uint16_t height) -> Window {
  return m_native->createWindow(width, height);
}

} // namespace tria::pal

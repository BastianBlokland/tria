#pragma once
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace tria::pal {

class NativePlatform;

class Platform final {
public:
  Platform(log::Logger* logger, std::string appName);
  Platform(const Platform& rhs)     = delete;
  Platform(Platform&& rhs) noexcept = default;
  ~Platform();

  auto operator=(const Platform& rhs) -> Platform& = delete;
  auto operator=(Platform&& rhs) noexcept -> Platform& = default;

  [[nodiscard]] auto getAppName() const noexcept -> std::string_view;

  auto handleEvents() -> void;

  [[nodiscard]] auto createWindow(uint16_t width, uint16_t height) -> Window;

private:
  std::unique_ptr<NativePlatform> m_native;
};

} // namespace tria::pal

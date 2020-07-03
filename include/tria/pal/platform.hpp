#pragma once
#include "tria/log/api.hpp"
#include "tria/pal/window.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace tria::pal {

class NativePlatform;

/*
 * Abstraction over a native platform.
 * Mainly related to managing windows and receiving os events.
 *
 * Note: Api is NOT thread-safe.
 */
class Platform final {
public:
  Platform(log::Logger* logger, std::string appName);
  Platform(const Platform& rhs)     = delete;
  Platform(Platform&& rhs) noexcept = default;
  ~Platform();

  auto operator=(const Platform& rhs) -> Platform& = delete;
  auto operator=(Platform&& rhs) noexcept -> Platform& = default;

  /* Get the name of this app.
   */
  [[nodiscard]] auto getAppName() const noexcept -> std::string_view;

  /* Handle any queued os events.
   * Should be called at a regular interval to be 'responsive' to the os.
   */
  auto handleEvents() -> void;

  /* Create a new os window.
   */
  [[nodiscard]] auto createWindow(uint16_t width, uint16_t height) -> Window;

private:
  std::unique_ptr<NativePlatform> m_native;
};

} // namespace tria::pal

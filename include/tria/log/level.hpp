#pragma once
#include <cstdint>
#include <string_view>

namespace tria::log {

using LevelMask = uint8_t;

enum class Level : uint8_t {
  Debug = 1U << 0U,
  Info  = 1U << 1U,
  Warn  = 1U << 2U,
  Error = 1U << 3U,
};

/* Mask that lets all logging levels go through.
 */
[[nodiscard]] constexpr auto allLevelMask() noexcept -> LevelMask {
  return ~static_cast<uint8_t>(0U);
}

/* Mask that blocks all logging levels.
 */
[[nodiscard]] constexpr auto noneLevelMask() noexcept -> LevelMask { return 0U; }

[[nodiscard]] constexpr auto levelMask(Level lvl) noexcept -> LevelMask {
  return static_cast<LevelMask>(lvl);
}

[[nodiscard]] constexpr auto operator|(Level lhs, Level rhs) noexcept -> LevelMask {
  return levelMask(lhs) | levelMask(rhs);
}

[[nodiscard]] constexpr auto operator|(LevelMask lhs, Level rhs) noexcept -> LevelMask {
  return lhs | levelMask(rhs);
}

[[nodiscard]] constexpr auto isInMask(LevelMask mask, Level lvl) noexcept {
  return (mask & static_cast<uint8_t>(lvl)) != 0;
}

[[nodiscard]] constexpr auto getName(Level lvl) noexcept -> std::string_view {
  switch (lvl) {
  case Level::Debug:
    return "dbg";
  case Level::Info:
    return "inf";
  case Level::Warn:
    return "wrn";
  case Level::Error:
    return "err";
  }
  return "unk";
}

} // namespace tria::log

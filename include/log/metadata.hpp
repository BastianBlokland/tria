#pragma once
#include "log/level.hpp"
#include <string_view>

namespace log {

/*
 * Static metadata about a log message. Can be constructed at compile-time (constexpr) and stored
 * in static memory in the binary.
 */
class MetaData final {
public:
  constexpr MetaData(
      Level lvl,
      std::string_view txt,
      std::string_view file,
      std::string_view func,
      uint32_t line) :
      m_lvl{lvl}, m_txt{txt}, m_file{file}, m_func{func}, m_line{line} {}

  MetaData(const MetaData& rhs)     = delete;
  MetaData(MetaData&& rhs) noexcept = delete;

  auto operator=(const MetaData& rhs) -> MetaData& = delete;
  auto operator=(MetaData&& rhs) noexcept -> MetaData& = delete;

  [[nodiscard]] constexpr auto getLevel() const noexcept { return m_lvl; }
  [[nodiscard]] constexpr auto getTxt() const noexcept { return m_txt; }
  [[nodiscard]] constexpr auto getFile() const noexcept { return m_file; }
  [[nodiscard]] constexpr auto getFunc() const noexcept { return m_func; }
  [[nodiscard]] constexpr auto getLine() const noexcept { return m_line; }

private:
  Level m_lvl;
  std::string_view m_txt;
  std::string_view m_file;
  std::string_view m_func;
  uint32_t m_line;
};

} // namespace log

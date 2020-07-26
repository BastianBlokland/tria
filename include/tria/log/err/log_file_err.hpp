#pragma once
#include "tria/fs.hpp"
#include <exception>
#include <string>
#include <string_view>

namespace tria::log::err {

/*
 * Exception that is thrown when an error occurs while creating a log file.
 */
class LogFileErr final : public std::exception {
public:
  LogFileErr() = delete;
  LogFileErr(const fs::path& path, std::string_view msg) :
      m_path{path},
      m_innerMsg{msg},
      m_msg{std::string{"Failed to create log file: "} + std::string{msg} + ": " +
            m_path.string()} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

  [[nodiscard]] auto getInnerMsg() const noexcept -> const std::string_view { return m_innerMsg; }

  [[nodiscard]] auto getPath() const noexcept -> const fs::path& { return m_path; }

private:
  fs::path m_path;
  std::string m_innerMsg;
  std::string m_msg;
};

} // namespace tria::log::err

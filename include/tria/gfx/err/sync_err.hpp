#pragma once
#include <exception>
#include <string>

namespace tria::gfx::err {

/*
 * Exception that is thrown when api is called when its not allowed.
 */
class SyncErr final : public std::exception {
public:
  SyncErr() = delete;
  SyncErr(const std::string& msg) : m_msg{std::string{"Sync error: "} + msg} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::gfx::err

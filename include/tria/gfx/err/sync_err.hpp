#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::gfx::err {

/*
 * Exception that is thrown when api is called when its not allowed.
 */
class SyncErr final : public std::exception {
public:
  SyncErr() = delete;
  SyncErr(std::string_view msg) : m_msg{std::string{"Sync error: "} + std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::gfx::err

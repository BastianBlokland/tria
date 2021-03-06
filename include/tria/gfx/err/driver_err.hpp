#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::gfx::err {

/*
 * Exception that is thrown when there is an error from the graphics driver.
 */
class DriverErr final : public std::exception {
public:
  DriverErr() = delete;
  DriverErr(std::string_view msg) : m_msg{std::string{"Gfx driver error: "} + std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::gfx::err

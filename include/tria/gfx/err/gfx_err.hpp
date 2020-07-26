#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::gfx::err {

/*
 * Exception that is thrown when an unrecoverable error occurs in the graphics library.
 */
class GfxErr final : public std::exception {
public:
  GfxErr() = delete;
  GfxErr(std::string_view msg) : m_msg{std::string{"Gfx error: "} + std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::gfx::err

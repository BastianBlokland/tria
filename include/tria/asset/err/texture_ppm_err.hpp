#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when a ppm texture is malformed.
 */
class TexturePpmErr final : public std::exception {
public:
  TexturePpmErr() = delete;
  TexturePpmErr(std::string_view msg) : m_msg{std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::asset::err

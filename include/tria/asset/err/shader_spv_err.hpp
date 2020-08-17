#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when a spir-v shader is malformed.
 */
class ShaderSpvErr final : public std::exception {
public:
  ShaderSpvErr() = delete;
  ShaderSpvErr(std::string_view msg) : m_msg{std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::asset::err

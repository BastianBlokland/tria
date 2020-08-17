#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when invalid json is read.
 */
class JsonErr final : public std::exception {
public:
  JsonErr() = delete;
  JsonErr(std::string_view msg) : m_msg{std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::asset::err

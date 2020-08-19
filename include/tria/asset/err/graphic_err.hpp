#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when a graphic definition is malformed.
 */
class GraphicErr final : public std::exception {
public:
  GraphicErr() = delete;
  GraphicErr(std::string_view msg) : m_msg{std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::asset::err

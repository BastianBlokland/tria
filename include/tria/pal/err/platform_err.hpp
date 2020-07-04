#pragma once
#include <exception>
#include <string>

namespace tria::pal::err {

/*
 * Exception that is thrown when there is an error from the native platform.
 */
class PlatformErr final : public std::exception {
public:
  PlatformErr() = delete;
  PlatformErr(unsigned long platformCode, const std::string& platformMsg) :
      m_platformCode{platformCode}, m_platformMsg{std::string{"Platform error: "} + platformMsg} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_platformMsg.c_str(); }

  [[nodiscard]] auto getPlatformCode() const noexcept { return m_platformCode; }

private:
  unsigned long m_platformCode;
  std::string m_platformMsg;
};

} // namespace tria::pal::err

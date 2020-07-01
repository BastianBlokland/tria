#pragma once
#include <exception>
#include <string>

namespace tria::pal::err {

class DisplayProtocolErr final : public std::exception {
public:
  DisplayProtocolErr() = delete;
  DisplayProtocolErr(unsigned long platformCode, std::string platformMsg) :
      m_platformCode{platformCode}, m_platformMsg{std::move(platformMsg)} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_platformMsg.c_str(); }

  [[nodiscard]] auto getPlatformCode() const noexcept -> unsigned long { return m_platformCode; }

private:
  unsigned long m_platformCode;
  std::string m_platformMsg;
};

} // namespace tria::pal::err

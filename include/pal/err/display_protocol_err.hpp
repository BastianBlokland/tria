#pragma once
#include <exception>

namespace pal::err {

class DisplayProtocolErr final : public std::exception {
public:
  DisplayProtocolErr() = delete;
  explicit DisplayProtocolErr(int platformCode) : m_platformCode{platformCode} {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return "Error in platform display protocol";
  }

  [[nodiscard]] auto getPlatformCode() const noexcept -> int { return m_platformCode; }

private:
  int m_platformCode;
};

} // namespace pal::err

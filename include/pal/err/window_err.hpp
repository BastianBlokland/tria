#pragma once
#include <exception>

namespace pal::err {

class WindowErr final : public std::exception {
public:
  [[nodiscard]] auto what() const noexcept -> const char* override { return "Invalid window"; }
};

} // namespace pal::err

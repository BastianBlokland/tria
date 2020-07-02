#pragma once
#include <exception>

namespace tria::pal::err {

class WindowErr final : public std::exception {
public:
  [[nodiscard]] auto what() const noexcept -> const char* override { return "Invalid window"; }
};

} // namespace tria::pal::err

#pragma once
#include <string>
#include <string_view>

namespace pal {

auto setThreadName(std::string_view name) noexcept -> bool;

[[nodiscard]] auto getThreadName() noexcept -> std::string;

} // namespace pal

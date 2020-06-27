#pragma once
#include "fs.hpp"
#include <string>
#include <string_view>

namespace pal {

auto getCurExecutablePath() noexcept -> fs::path;

auto setThreadName(std::string_view name) noexcept -> bool;

[[nodiscard]] auto getThreadName() noexcept -> std::string;

} // namespace pal

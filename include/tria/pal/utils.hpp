#pragma once
#include "tria/fs.hpp"
#include <string>
#include <string_view>

namespace tria::pal {

auto getCurExecutablePath() noexcept -> fs::path;

auto setThreadName(std::string_view name) noexcept -> bool;

[[nodiscard]] auto getThreadName() noexcept -> std::string;

} // namespace tria::pal

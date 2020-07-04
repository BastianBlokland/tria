#pragma once
#include "tria/fs.hpp"
#include <string>
#include <string_view>

namespace tria::pal {

/* Get the path to the currently running executable.
 */
auto getCurExecutablePath() noexcept -> fs::path;

/* Get the name of the currently running executable.
 */
auto getCurExecutableName() noexcept -> std::string;

/* Assign a name to the current thread.
 * Mostly usefull for debug purposes.
 * Note: Not all platforms implement this feature (returns false if setting the name fails).
 */
auto setThreadName(std::string_view name) noexcept -> bool;

/* Get a name assigned to the current thread.
 * Returns an empty string if no name was assigned or this platform does not support naming
 * threads.
 */
[[nodiscard]] auto getThreadName() noexcept -> std::string;

} // namespace tria::pal

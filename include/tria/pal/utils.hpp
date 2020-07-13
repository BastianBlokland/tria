#pragma once
#include "tria/fs.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace tria::pal {

/* Get the path to the currently running executable.
 */
auto getCurExecutablePath() noexcept -> fs::path;

/* Get the name of the currently running executable.
 */
auto getCurExecutableName() noexcept -> std::string;

/* Get the id of the current process.
 */
auto getCurProcessId() noexcept -> int64_t;

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

/* Setup the console (if attached) for console output.
 * Returns true if a console is present or false if no console is present (for example if the output
 * is redirected to a file).
 */
auto setupConsole() noexcept -> bool;

} // namespace tria::pal

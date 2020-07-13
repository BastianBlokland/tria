#pragma once

namespace tria::pal {

/* Capture interrupt (ctrl-c) events. After setting up the handler 'isInterruptRequested' can be
 * used to query if the user wants to interrupt the program.
 * Return value indicates if a interrupt handler was successfully setup.
 */
auto setupInterruptHandler() noexcept -> bool;

/* Did the user request to interrupt the program (ctrl-c).
 * Note: Requires having the handler setup with 'setupInterruptHandler'.
 */
[[nodiscard]] auto isInterruptRequested() noexcept -> bool;

} // namespace tria::pal

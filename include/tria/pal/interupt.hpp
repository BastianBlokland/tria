#pragma once

namespace tria::pal {

/* Capture interupt (ctrl-c) events. After setting up the handler 'isInteruptRequested' can be used
 * to query if the user wants to interupt the program.
 * Return value indicates if a interupt handler was successfully setup.
 */
auto setupInteruptHandler() noexcept -> bool;

/* Did the user request to interupt the program (ctrl-c).
 * Note: Requires having the handler setup with 'setupInteruptHandler'.
 */
[[nodiscard]] auto isInteruptRequested() noexcept -> bool;

} // namespace tria::pal

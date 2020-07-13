#include "tria/pal/interupt.hpp"
#include <atomic>
#include <windows.h>

namespace tria::pal {

std::atomic_bool g_interuptRequested = false;

auto interuptHandler(DWORD dwCtrlType) -> int {
  switch (dwCtrlType) {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
    g_interuptRequested.store(true, std::memory_order_release);
    return true; // Indicate that we have handled the event.
  default:
    return false; // Indicate that we have not handled the event.
  }
}

auto setupInteruptHandler() noexcept -> bool {
  return SetConsoleCtrlHandler(interuptHandler, true);
}

auto isInteruptRequested() noexcept -> bool {
  return g_interuptRequested.load(std::memory_order_acquire);
}

} // namespace tria::pal

#include "tria/pal/interrupt.hpp"
#include <atomic>
#include <windows.h>

namespace tria::pal {

std::atomic_bool g_interruptRequested = false;

auto interruptHandler(DWORD dwCtrlType) noexcept -> int {
  switch (dwCtrlType) {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
    g_interruptRequested.store(true, std::memory_order_release);
    return true; // Indicate that we have handled the event.
  default:
    return false; // Indicate that we have not handled the event.
  }
}

auto setupInterruptHandler() noexcept -> bool {
  return SetConsoleCtrlHandler(interruptHandler, true);
}

auto isInterruptRequested() noexcept -> bool {
  return g_interruptRequested.load(std::memory_order_acquire);
}

} // namespace tria::pal

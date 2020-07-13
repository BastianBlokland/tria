#include "tria/pal/interrupt.hpp"
#include <atomic>
#include <signal.h>

namespace tria::pal {

std::atomic_bool g_interruptRequested = false;

auto interruptHandler(int /*unused*/) -> void {
  g_interruptRequested.store(true, std::memory_order_release);
}

auto setupInterruptHandler() noexcept -> bool {

  struct sigaction actionInfo = {};
  actionInfo.sa_handler       = interruptHandler;
  sigemptyset(&actionInfo.sa_mask);

  return sigaction(SIGINT, &actionInfo, nullptr) == 0;
}

auto isInterruptRequested() noexcept -> bool {
  return g_interruptRequested.load(std::memory_order_acquire);
}

} // namespace tria::pal

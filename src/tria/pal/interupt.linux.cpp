#include "tria/pal/interupt.hpp"
#include <atomic>
#include <signal.h>

namespace tria::pal {

std::atomic_bool g_interuptRequested = false;

auto interuptHandler(int /*unused*/) -> void {
  g_interuptRequested.store(true, std::memory_order_release);
}

auto setupInteruptHandler() noexcept -> bool {

  struct sigaction actionInfo = {};
  actionInfo.sa_handler       = interuptHandler;
  sigemptyset(&actionInfo.sa_mask);

  return sigaction(SIGINT, &actionInfo, nullptr) == 0;
}

auto isInteruptRequested() noexcept -> bool {
  return g_interuptRequested.load(std::memory_order_acquire);
}

} // namespace tria::pal

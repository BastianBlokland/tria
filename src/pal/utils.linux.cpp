#include "pal/utils.hpp"
#include <cstring>
#include <pthread.h>

namespace pal {

auto setThreadName(std::string_view name) noexcept -> bool {
  const auto curThread = pthread_self();
  return pthread_setname_np(curThread, name.data()) == 0;
}

[[nodiscard]] auto getThreadName() noexcept -> std::string {
  constexpr auto maxSize = 16U;

  auto result = std::string{};
  result.resize(maxSize);

  const auto curThread = pthread_self();
  if (pthread_getname_np(curThread, result.data(), maxSize) != 0) {
    return {};
  }

  result.resize(std::strlen(result.data()));
  return result;
}

} // namespace pal

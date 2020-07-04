#include "tria/pal/utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

namespace tria::pal {

static auto errorExit(const char* msg) {
  std::fprintf(stderr, "%s\n", msg);
  std::fflush(stderr);
  std::exit(EXIT_FAILURE);
}

auto getCurExecutablePath() noexcept -> fs::path {
  constexpr auto selfLink = "/proc/self/exe";

  std::error_code err;
  auto path = fs::read_symlink(selfLink, err);
  if (err) {
    errorExit("Failed to read '/proc/self/exe'");
  }
  return path;
}

auto getCurExecutableName() noexcept -> std::string {
  return getCurExecutablePath().filename().string();
}

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

} // namespace tria::pal

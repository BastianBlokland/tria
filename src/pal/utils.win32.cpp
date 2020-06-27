#include "pal/utils.hpp"
#include <codecvt>
#include <locale>
#include <windows.h>

namespace pal {

/* Since Windows 10 version 1607 there is an api to name threads. If our windows version is older
 * or we are compiling with MinGW (which unfortunately at the time of writing doesn't have this api
 * yet) then these functions compile into no-ops.
 */
#if !defined(__MINGW32__) && NTDDI_VERSION >= NTDDI_WIN10_RS1

auto setThreadName(std::string_view name) noexcept -> bool {
  const auto curThread = GetCurrentThread();

  auto wideName      = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name.data());
  const auto retCode = SetThreadDescription(curThread, wideName.data());
  return !FAILED(retCode);
}

[[nodiscard]] auto getThreadName() noexcept -> std::string {
  const auto curThread = GetCurrentThread();

  PWSTR wideName     = nullptr;
  const auto retCode = GetThreadDescription(curThread, &wideName);

  auto result = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wideName);

  LocalFree(wideName);
  return result;
}

#else

auto setThreadName(std::string_view /*unused*/) noexcept -> bool { return false; }

[[nodiscard]] auto getThreadName() noexcept -> std::string { return "thread"; }

#endif

} // namespace pal

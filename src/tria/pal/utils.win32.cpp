#include "tria/pal/utils.hpp"
#include <array>
#include <codecvt>
#include <locale>
#include <windows.h>

namespace tria::pal {

namespace {

auto errorExit(const char* msg) {
  std::fprintf(stderr, "%s\n", msg);
  std::fflush(stderr);
  std::exit(EXIT_FAILURE);
}

auto endsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
      str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

auto enableVTConsoleMode() -> bool {
  auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE) {
    return false;
  }
  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode)) {
    return false;
  }
  dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  return SetConsoleMode(hOut, dwMode);
}

} // namespace

auto setupConsole() noexcept -> bool {
  // Enable vt-console mode to support ansi escape sequences.
  return enableVTConsoleMode();
}

auto getCurExecutablePath() noexcept -> fs::path {
  auto pathBuffer = std::array<char, MAX_PATH>{};
  auto size       = GetModuleFileName(nullptr, pathBuffer.data(), MAX_PATH);
  if (size == 0) {
    errorExit("Failed to retreive executable filename");
  }
  return fs::path{pathBuffer.data()};
}

auto getCurExecutableName() noexcept -> std::string {
  auto fileName = getCurExecutablePath().filename().string();
  if (endsWith(fileName, ".exe")) {
    fileName.erase(fileName.end() - 4, fileName.end());
  }
  return fileName;
}

auto getCurProcessId() noexcept -> int64_t { return GetCurrentProcessId(); }

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
  if (FAILED(retCode)) {
    return "";
  }

  auto result = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wideName);

  LocalFree(wideName);
  return result;
}

#else

auto setThreadName(std::string_view /*unused*/) noexcept -> bool { return false; }

[[nodiscard]] auto getThreadName() noexcept -> std::string { return ""; }

#endif

} // namespace tria::pal

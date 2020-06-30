#pragma once
#include <array>
#include <chrono>
#include <log/level.hpp>
#include <string>

namespace log::internal {

inline auto writeLong(std::string* str, long value) noexcept -> void {
  constexpr auto maxCharSize = 21;
  auto buffer                = std::array<char, maxCharSize>{};

  // Write the characters using either std::to_chars to std::snprintf.
#if defined(HAS_CHAR_CONV)
  const auto convRes = std::to_chars(buffer.begin(), buffer.end(), value);
  const auto size    = convRes.ptr - buffer.data();
#else
  const auto size = std::snprintf(buffer.data(), maxCharSize, "%ld", value);
#endif

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer.data(), static_cast<std::string_view::size_type>(size)});
}

inline auto writeDouble(std::string* str, double value) noexcept -> void {

  // Check how many characters we need to represent the double.
  const auto size = std::snprintf(nullptr, 0, "%.10g", value);

  // Allocate a buffer on the stack with that size.
  auto buffer = static_cast<char*>(alloca(size + 1));

  // Write the characters in the stack buffer.
  std::snprintf(buffer, size + 1, "%.10g", value);

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer, static_cast<std::string_view::size_type>(size)});
}

inline auto writeLvl(std::string* str, Level value) noexcept -> void {
  str->append(getName(value));
}

/* ISO 8601 in UTC with fractions of seconds (https://en.wikipedia.org/wiki/ISO_8601)
 * Example output: 2020-06-30T18:15:49.199029728Z
 */
inline auto writeIsoTime(std::string* str, std::chrono::system_clock::time_point value) noexcept
    -> void {

  using namespace std::chrono;

  static_assert(
      system_clock::time_point::period::den == 1'000'000'000 &&
      system_clock::time_point::period::num == 1);
  auto time = std::chrono::system_clock::to_time_t(value);

  constexpr auto bufferSize = 31;
  auto buffer               = std::array<char, bufferSize>{};

  // Write the characters up to seconds into the buffer.
  std::strftime(buffer.data(), bufferSize, "%Y-%m-%dT%H:%M:%S.", std::gmtime(&time));

  // Write the fractions of a seconds into the buffer.
  const auto countSinceEpoch = static_cast<long long>(value.time_since_epoch().count());
  std::sprintf(buffer.data() + 20, "%09lld", countSinceEpoch % 1'000'000'000);

  buffer[29] = 'Z';  // Timezone indicator (Z = UTC).
  buffer[30] = '\0'; // Null-terminate.

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer.data(), static_cast<std::string_view::size_type>(30)});
}

} // namespace log::internal

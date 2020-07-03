#pragma once
#include <array>
#include <chrono>
#include <ctime>
#include <string>
#include <tria/log/level.hpp>
#include <type_traits>

namespace tria::log::internal {

template <typename>
constexpr bool falseValue = false;

template <typename T>
constexpr auto getSecondsFracFormatSpecifier() noexcept -> const char* {
  if constexpr (std::is_same<T, int>::value) {
    return "%06d";
  } else if constexpr (std::is_same<T, long>::value) {
    return "%06ld";
  } else if constexpr (std::is_same<T, long long>::value) {
    return "%06lld";
  } else {
    static_assert(falseValue<T>, "Unsupported type");
    return nullptr;
  }
}

inline auto writeLong(std::string* str, long value) noexcept {
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

inline auto writeDouble(std::string* str, double value) noexcept {

  // Check how many characters we need to represent the double.
  const auto size = std::snprintf(nullptr, 0, "%.10g", value);

  // Allocate a buffer on the stack with that size.
  auto buffer = static_cast<char*>(alloca(size + 1));

  // Write the characters in the stack buffer.
  std::snprintf(buffer, size + 1, "%.10g", value);

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer, static_cast<std::string_view::size_type>(size)});
}

inline auto writeLvl(std::string* str, Level value) noexcept { str->append(getName(value)); }

/* ISO 8601 in UTC with microseconds (https://en.wikipedia.org/wiki/ISO_8601).
 * Example output: 2020-06-30T18:15:49.199029Z
 */
inline auto writeIsoTime(std::string* str, std::chrono::system_clock::time_point value) noexcept {

  using namespace std::chrono;

  const auto time         = system_clock::to_time_t(value);
  const auto remMicroSecs = duration_cast<microseconds>(value - system_clock::from_time_t(time));

  constexpr auto bufferSize = 28;
  auto buffer               = std::array<char, bufferSize>{};

  // Write the characters up to seconds into the buffer.
  std::strftime(buffer.data(), bufferSize, "%Y-%m-%dT%H:%M:%S.", std::gmtime(&time));

  // Write the fractions of a seconds into the buffer.
  std::sprintf(
      buffer.data() + 20,
      getSecondsFracFormatSpecifier<decltype(remMicroSecs)::rep>(),
      remMicroSecs.count());

  buffer[26] = 'Z';  // Timezone indicator (Z = UTC).
  buffer[27] = '\0'; // Null-terminate.

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(
      std::string_view{buffer.data(), static_cast<std::string_view::size_type>(bufferSize - 1)});
}

} // namespace tria::log::internal

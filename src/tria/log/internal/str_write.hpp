#pragma once
#include "tria/log/level.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <string>
#include <type_traits>

namespace tria::log::internal {

template <typename>
constexpr bool falseValue = false;

template <typename T>
constexpr auto getIntFormatSpecifier() noexcept -> const char* {
  if constexpr (std::is_same_v<T, int>) {
    return "%d";
  } else if constexpr (std::is_same_v<T, unsigned int>) {
    return "%u";
  } else if constexpr (std::is_same_v<T, long>) {
    return "%ld";
  } else if constexpr (std::is_same_v<T, unsigned long>) {
    return "%lu";
  } else if constexpr (std::is_same_v<T, long long>) {
    return "%lld";
  } else if constexpr (std::is_same_v<T, unsigned long long>) {
    return "%llu";
  } else {
    static_assert(falseValue<T>, "Unsupported type");
    return nullptr;
  }
}

template <typename T>
constexpr auto getSecondsFracFormatSpecifier() noexcept -> const char* {
  if constexpr (std::is_same_v<T, int>) {
    return "%06d";
  } else if constexpr (std::is_same_v<T, long>) {
    return "%06ld";
  } else if constexpr (std::is_same_v<T, long long>) {
    return "%06lld";
  } else {
    static_assert(falseValue<T>, "Unsupported type");
    return nullptr;
  }
}

template <typename IntType>
inline auto writeInt(std::string* str, IntType value) noexcept {
  constexpr auto maxCharSize = 21;
  auto buffer                = std::array<char, maxCharSize>{};

  // Write the characters using either std::to_chars to std::snprintf.
#if defined(HAS_CHAR_CONV)
  const auto convRes = std::to_chars(buffer.begin(), buffer.end(), value);
  const auto size    = convRes.ptr - buffer.data();
#else
  const auto size =
      std::snprintf(buffer.data(), maxCharSize, getIntFormatSpecifier<IntType>(), value);
#endif

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer.data(), static_cast<std::string_view::size_type>(size)});
}

inline auto writeDouble(std::string* str, double value, const char* fmtStr = "%.10g") noexcept {

  // Check how many characters we need to represent the double.
  const auto size = std::snprintf(nullptr, 0, fmtStr, value);

  // Allocate a buffer on the stack with that size.
  auto buffer = static_cast<char*>(alloca(size + 1));

  // Write the characters in the stack buffer.
  std::snprintf(buffer, size + 1, fmtStr, value);

  // Wrap the buffer into a string_view and call the string_view overload.
  str->append(std::string_view{buffer, static_cast<std::string_view::size_type>(size)});
}

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

inline auto writePrettyDuration(std::string* str, std::chrono::duration<double> dur) noexcept {
  constexpr static std::array<std::string_view, 4> units = {
      " sec",
      " ms",
      " us",
      " ns",
  };

  auto unitIdx = 0U;
  auto t       = dur.count();
  for (; t < 1.0 && unitIdx != units.size() - 1; ++unitIdx) {
    t *= 1000.0;
  }
  const auto rounded = std::round(t);
  if (std::abs(t - rounded) < .05) {
    writeInt(str, static_cast<uint64_t>(rounded));
  } else {
    writeDouble(str, t, "%.1f");
  }
  str->append(units[unitIdx]);
}

inline auto writePrettyMemSize(std::string* str, const size_t size) noexcept {
  constexpr static std::array<std::string_view, 6> units = {
      " B",
      " KiB",
      " MiB",
      " GiB",
      " TiB",
      " PiB",
  };

  auto unitIdx = 0U;
  auto sizeD   = static_cast<double>(size);
  for (; sizeD >= 1024.0 && unitIdx != units.size() - 1; ++unitIdx) {
    sizeD /= 1024.0;
  }
  if (sizeD - std::floor(sizeD) < .1) {
    writeInt(str, static_cast<uint64_t>(sizeD));
  } else {
    writeDouble(str, sizeD, "%.1f");
  }
  str->append(units[unitIdx]);
}

} // namespace tria::log::internal

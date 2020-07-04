#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace tria::log {

/*
 * Runtime parameter to a log message.
 * Supported types:
 * - Integer types (stored in a signed/unsigned 64 bit integer).
 * - Floating point types (float and double, stored as a double).
 * - Strings (stored as a copy).
 *
 * Note: Keys should be literals or strings that have a longer lifetime then the logger.
 * Note: Because it can store std::string it should be moved whenever possible.
 */
class Param final {
public:
  Param() = delete;

  template <
      typename T,
      std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void*> = nullptr>
  Param(std::string_view key, T value) noexcept :
      m_key{key}, m_value{static_cast<int64_t>(value)} {}

  template <
      typename T,
      std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void*> = nullptr>
  Param(std::string_view key, T value) noexcept :
      m_key{key}, m_value{static_cast<uint64_t>(value)} {}

  Param(std::string_view key, double value) noexcept : m_key{key}, m_value{value} {}

  Param(std::string_view key, std::string value) noexcept;

  Param(const Param& rhs)     = default;
  Param(Param&& rhs) noexcept = default;

  auto operator=(const Param& rhs) -> Param& = default;
  auto operator=(Param&& rhs) noexcept -> Param& = default;

  auto operator==(const Param& rhs) const noexcept -> bool;
  auto operator!=(const Param& rhs) const noexcept -> bool;

  [[nodiscard]] constexpr auto getKey() const noexcept { return m_key; }

  auto writeValue(std::string* tgtStr) const noexcept -> void;

private:
  using ValueType = std::variant<int64_t, uint64_t, double, std::string>;

  std::string_view m_key;
  ValueType m_value;
};

} // namespace tria::log

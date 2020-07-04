#pragma once
#include <string>
#include <string_view>
#include <variant>

namespace tria::log {

/*
 * Runtime parameter to a log message.
 * Supported types:
 * - Integer types (stored in a signed 64 bit integer).
 *    TODO: Support unsigned 64 bit integers > signed 64 max?
 * - Floating point types (float and double, stored as a double).
 * - Strings (stored as a copy).
 *
 * Note: Keys should be literals or strings that have a longer lifetime then the logger.
 * Note: Because it can store std::string it should be moved whenever possible.
 */
class Param final {
public:
  Param() = delete;

  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  Param(std::string_view key, T value) noexcept : m_key{key}, m_value{static_cast<long>(value)} {}

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
  using ValueType = std::variant<long, double, std::string>;

  std::string_view m_key;
  ValueType m_value;
};

} // namespace tria::log

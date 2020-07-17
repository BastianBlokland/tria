#pragma once
#include <chrono>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace tria::log {

/* Elapsed time.
 */
using Duration = std::chrono::duration<double>;

/* Point in time.
 */
using TimePoint = std::chrono::system_clock::time_point;

/* Memory size.
 * Wrapper around a size_t, that gives additional semantic information.
 */
class MemSize final {
public:
  explicit MemSize(size_t size) : m_size{size} {}

  auto operator==(const MemSize& rhs) const noexcept -> bool { return m_size == rhs.m_size; }

  [[nodiscard]] auto getSize() const noexcept { return m_size; }

private:
  size_t m_size;
};

/* Supported output mode for writing a value.
 */
enum class ParamWriteMode {
  Pretty,
  Json,
};

/* Value of a log parameter.
 * Supported types:
 * - Integer types (stored in a signed/unsigned 64 bit integer).
 * - Floating point types (float and double, stored as a double).
 * - Bool.
 * - String (stored as a copy).
 * - Duration (std::chrono::duration<double>).
 * - TimePoint (std::chrono::system_clock::time_point).
 * - MemSize (wrapper around size_t).
 *
 * Note: Because it can store std::string it should be moved whenever possible.
 */
class Value final {
public:
  Value() = delete;

  template <
      typename T,
      std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void*> = nullptr>
  Value(T value) noexcept : m_val{static_cast<int64_t>(value)} {}

  template <
      typename T,
      std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void*> = nullptr>
  Value(T value) noexcept : m_val{static_cast<uint64_t>(value)} {}

  Value(bool value) noexcept : m_val{value} {}

  Value(double value) noexcept : m_val{value} {}

  Value(const char* value) noexcept : Value(std::string(value)) {}

  Value(std::string_view value) noexcept : Value(std::string(value)) {}

  Value(std::string value) noexcept;

  Value(Duration value) noexcept : m_val{value} {}

  Value(TimePoint value) noexcept : m_val{value} {}

  Value(MemSize value) noexcept : m_val{value} {}

  Value(const Value& rhs)     = default;
  Value(Value&& rhs) noexcept = default;

  auto operator=(const Value& rhs) -> Value& = default;
  auto operator=(Value&& rhs) noexcept -> Value& = default;

  auto operator==(const Value& rhs) const noexcept -> bool;
  auto operator!=(const Value& rhs) const noexcept -> bool;

  auto write(std::string* tgtStr, ParamWriteMode mode) const noexcept -> void;

private:
  using ValueType =
      std::variant<int64_t, uint64_t, double, bool, std::string, Duration, TimePoint, MemSize>;

  ValueType m_val;
};

/* Factory that constructs a Value (or a std::vector<Value>) from an arbitrary type.
 * Designed to be an extension point by specializing for a custom type.
 */
template <typename Type>
struct ValueFactory final {
  [[nodiscard]] auto operator()(Type&& t) const noexcept -> Value { return Value{std::move(t)}; }
};

/* Parameter of a log message.
 * Note: Keys should be literals or strings that have a longer lifetime then the logger.
 * Note: Because it can store strings or vectors it should be moved whenever possible.
 */
class Param final {
public:
  Param() = delete;

  template <typename T>
  Param(std::string_view key, T&& rawValue, ValueFactory<T> factory = {}) noexcept :
      m_key{key}, m_value{factory(std::forward<T>(rawValue))} {}

  template <typename... RawValues>
  Param(std::string_view key, RawValues&&... rawValues) noexcept :
      m_key{key}, m_value{std::vector<Value>{Value{std::forward<RawValues>(rawValues)}...}} {}

  Param(const Param& rhs)     = default;
  Param(Param&& rhs) noexcept = default;

  auto operator=(const Param& rhs) -> Param& = default;
  auto operator=(Param&& rhs) noexcept -> Param& = default;

  auto operator==(const Param& rhs) const noexcept -> bool;
  auto operator!=(const Param& rhs) const noexcept -> bool;

  [[nodiscard]] constexpr auto getKey() const noexcept { return m_key; }

  auto writeValue(std::string* tgtStr, ParamWriteMode mode) const noexcept -> void;

private:
  using ValueType = std::variant<Value, std::vector<Value>>;

  std::string_view m_key;
  ValueType m_value;
};

} // namespace tria::log

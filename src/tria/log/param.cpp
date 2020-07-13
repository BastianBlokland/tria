#include "tria/log/param.hpp"
#include "internal/str_write.hpp"
#include <cassert>

namespace tria::log {

namespace {

auto escape(std::string&& str) -> std::string {
  for (auto i = 0U;; ++i) {
    switch (str[i]) {
    case '\r':
      str.replace(i, 1, "\\r");
      ++i;
      break;
    case '\n':
      str.replace(i, 1, "\\n");
      ++i;
      break;
    case '\t':
      str.replace(i, 1, "\\t");
      ++i;
      break;
    case '\0':
      goto End;
    default:
      break;
    }
  }
End:
  return std::move(str);
}

} // namespace

template <typename>
constexpr bool falseValue = false;

Param::Param(std::string_view key, std::string value) noexcept :
    m_key{key}, m_value{escape(std::move(value))} {}

auto Param::operator==(const Param& rhs) const noexcept -> bool {
  return m_key == rhs.m_key && m_value == rhs.m_value;
}

auto Param::operator!=(const Param& rhs) const noexcept -> bool { return !Param::operator==(rhs); }

auto Param::writeValue(std::string* tgtStr, WriteMode mode) const noexcept -> void {
  assert(tgtStr);
  std::visit(
      [tgtStr, mode](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
          internal::writeInt(tgtStr, arg);
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, double>) {
          internal::writeDouble(tgtStr, arg);
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, bool>) {
          tgtStr->append(arg ? "true" : "false");
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, std::string>) {
          if (mode == WriteMode::Json) {
            tgtStr->append("\"");
          }
          tgtStr->append(arg);
          if (mode == WriteMode::Json) {
            tgtStr->append("\"");
          }
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, Duration>) {
          switch (mode) {
          case WriteMode::Json: {
            // In json write the elapsed nanoseconds.
            auto nanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(arg).count();
            internal::writeInt(tgtStr, nanoSec);
          } break;
          case WriteMode::Pretty:
            internal::writePrettyDuration(tgtStr, arg);
            break;
          }
        } else if constexpr (std::is_same_v<T, TimePoint>) {
          if (mode == WriteMode::Json) {
            tgtStr->append("\"");
          }
          internal::writeIsoTime(tgtStr, arg);
          if (mode == WriteMode::Json) {
            tgtStr->append("\"");
          }
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, MemSize>)
          switch (mode) {
          case WriteMode::Json:
            // In json just write the size in bytes.
            internal::writeInt(tgtStr, arg.getSize());
            break;
          case WriteMode::Pretty:
            internal::writePrettyMemSize(tgtStr, arg.getSize());
            break;
          }
        else {
          static_assert(falseValue<T>, "Non exhaustive write-value routine");
        }
      },
      m_value);
}

} // namespace tria::log

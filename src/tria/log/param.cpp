#include "tria/log/param.hpp"
#include "internal/str_write.hpp"
#include <cassert>

namespace tria::log {

template <typename>
constexpr bool falseValue = false;

auto Value::operator==(const Value& rhs) const noexcept -> bool { return m_val == rhs.m_val; }

auto Value::operator!=(const Value& rhs) const noexcept -> bool { return !Value::operator==(rhs); }

auto Value::write(std::string* tgtStr, ParamWriteMode mode) const noexcept -> void {
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
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
          internal::writeStrEscaped(tgtStr, arg);
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, fs::path>) {
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
          internal::writePathNormalized(tgtStr, arg.c_str());
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, Duration>) {
          switch (mode) {
          case ParamWriteMode::Json: {
            // In json write the elapsed nanoseconds.
            auto nanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(arg).count();
            internal::writeInt(tgtStr, nanoSec);
          } break;
          case ParamWriteMode::Pretty:
            internal::writePrettyDuration(tgtStr, arg);
            break;
          }
        } else if constexpr (std::is_same_v<T, TimePoint>) {
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
          internal::writeIsoTime(tgtStr, arg);
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("\"");
          }
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, MemSize>)
          switch (mode) {
          case ParamWriteMode::Json:
            // In json just write the size in bytes.
            internal::writeInt(tgtStr, arg.getSize());
            break;
          case ParamWriteMode::Pretty:
            internal::writePrettyMemSize(tgtStr, arg.getSize());
            break;
          }
        else {
          static_assert(falseValue<T>, "Non exhaustive write-value routine");
        }
      },
      m_val);
}

auto Param::operator==(const Param& rhs) const noexcept -> bool {
  return m_key == rhs.m_key && m_value == rhs.m_value;
}

auto Param::operator!=(const Param& rhs) const noexcept -> bool { return !Param::operator==(rhs); }

auto Param::writeValue(std::string* tgtStr, ParamWriteMode mode) const noexcept -> void {
  assert(tgtStr);
  std::visit(
      [tgtStr, mode](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Value>) {

          arg.write(tgtStr, mode);

        } else if constexpr (std::is_same_v<T, std::vector<Value>>) {

          if (mode == ParamWriteMode::Json) {
            tgtStr->append("[");
          }
          for (auto itr = arg.begin(); itr != arg.end(); ++itr) {
            itr->write(tgtStr, mode);
            auto isLast = itr == arg.end() - 1;
            if (!isLast) {
              tgtStr->append(", ");
            }
          }
          if (mode == ParamWriteMode::Json) {
            tgtStr->append("]");
          }

        } else {
          static_assert(falseValue<T>, "Non exhaustive write-value routine");
        }
      },
      m_value);
}

} // namespace tria::log

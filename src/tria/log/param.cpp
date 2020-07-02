#include "tria/log/param.hpp"
#include "internal/str_write.hpp"
#include <cassert>

namespace tria::log {

template <typename>
constexpr bool falseValue = false;

auto Param::operator==(const Param& rhs) const noexcept -> bool {
  return m_key == rhs.m_key && m_value == rhs.m_value;
}

auto Param::operator!=(const Param& rhs) const noexcept -> bool { return !Param::operator==(rhs); }

auto Param::writeValue(std::string* tgtStr) const noexcept -> void {
  assert(tgtStr);
  std::visit(
      [tgtStr](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if constexpr (std::is_same_v<T, long>) {
          internal::writeLong(tgtStr, arg);
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, double>) {
          internal::writeDouble(tgtStr, arg);
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, std::string_view>) {
          tgtStr->append("\"");
          tgtStr->append(arg);
          tgtStr->append("\"");
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        else if constexpr (std::is_same_v<T, std::string>) {
          tgtStr->append("\"");
          tgtStr->append(arg);
          tgtStr->append("\"");
        } else {
          static_assert(falseValue<T>, "Non exhaustive write-value routine");
        }
      },
      m_value);
}

} // namespace tria::log

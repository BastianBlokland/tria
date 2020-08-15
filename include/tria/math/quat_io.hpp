#include "tria/math/quat.hpp"
#include <iostream>

namespace tria::math {

/* Specialization for iostream output.
 */
template <typename Type>
auto operator<<(std::ostream& out, const Quat<Type>& rhs) -> std::ostream& {
  out << "[";
  for (auto i = 0U; i != Quat<Type>::Size; ++i) {
    out << rhs[i];
    if (i < Quat<Type>::Size - 1) {
      out << ",";
    }
  }
  out << "]";
  return out;
}

} // namespace tria::math

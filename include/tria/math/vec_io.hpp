#include <iostream>
#include <tria/math/vec.hpp>

namespace tria::math {

/* Specialization for iostream output.
 */
template <typename Type, size_t Size>
auto operator<<(std::ostream& out, const tria::math::Vec<Type, Size>& rhs) -> std::ostream& {
  out << "[";
  for (auto i = 0U; i != Size; ++i) {
    out << rhs[i];
    if (i < Size - 1) {
      out << ",";
    }
  }
  out << "]";
  return out;
}

} // namespace tria::math

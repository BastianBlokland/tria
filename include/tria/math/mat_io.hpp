#include "tria/math/mat.hpp"
#include "tria/math/vec_io.hpp"
#include <iostream>

namespace tria::math {

/* Specialization for iostream output.
 */
template <typename Type, size_t Size>
auto operator<<(std::ostream& out, const Mat<Type, Size>& rhs) -> std::ostream& {
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

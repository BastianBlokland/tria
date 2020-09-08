#include "tria/math/box.hpp"
#include "tria/math/vec_io.hpp"
#include <iostream>

namespace tria::math {

/* Specialization for iostream output.
 */
template <typename Type, size_t Size>
auto operator<<(std::ostream& out, const Box<Type, Size>& rhs) -> std::ostream& {
  out << "[" << rhs.min() << "," << rhs.max() << "]";
  return out;
}

} // namespace tria::math

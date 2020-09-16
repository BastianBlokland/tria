#include "tria/log/param.hpp"
#include "tria/math/pod_vector.hpp"
#include <vector>

/* Specialize tria::log::ValueFactory so vectors can be used as log parameters.
 */
namespace tria::log {

template <typename Type>
struct ValueFactory<math::PodVector<Type>> final {
  [[nodiscard]] auto operator()(const math::PodVector<Type>& vec) const noexcept
      -> std::vector<Value> {
    auto result = std::vector<Value>();
    result.reserve(vec.size());
    for (const Type& v : vec) {
      result.emplace_back(v);
    }
    return result;
  }
};

} // namespace tria::log

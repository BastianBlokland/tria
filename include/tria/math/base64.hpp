#pragma once
#include "pod_vector.hpp"
#include <string_view>

namespace tria::math {

/* Decode the MIME Base64 encoded input.
 */
[[nodiscard]] auto base64Decode(std::string_view input) noexcept -> RawData;

} // namespace tria::math

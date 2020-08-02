#pragma once
#include "simdjson.h"
#include "tria/math/pod_vector.hpp"

namespace tria::asset::internal {

using JsonParseResult = simdjson::simdjson_result<simdjson::dom::element>;

/*
 * Parse a json file.
 * Note: Return value can be used until the next call to 'parseJson' on the same thread.
 * Safe to be called concurrently but results should not be shared among threads.
 * Note: Input buffer has to be padded with x bytes, current DatabaseImpl provides that guarantee.
 */
[[nodiscard]] auto parseJson(const math::RawData& raw) noexcept -> JsonParseResult;

} // namespace tria::asset::internal

#pragma once
#include "simdjson.h"
#include <vector>

namespace tria::asset::internal {

using JsonParseResult = simdjson::simdjson_result<simdjson::dom::element>;

/*
 * Parse a json file.
 * Note: Return value can be used until the next call to 'parseJson' on the same thread.
 * Safe to be called concurrently but results should not be shared among threads.
 */
[[nodiscard]] auto parseJson(const std::vector<char>& raw) noexcept -> JsonParseResult;

} // namespace tria::asset::internal

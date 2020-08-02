#include "json.hpp"

namespace tria::asset::internal {

auto parseJson(const math::RawData& raw) noexcept -> JsonParseResult {
  // Verify that the input is sufficiently padded.
  assert(raw.capacity() - raw.size() >= simdjson::SIMDJSON_PADDING);

  thread_local static simdjson::dom::parser parser;
  return parser.parse(raw.begin(), raw.size(), false);
}

} // namespace tria::asset::internal

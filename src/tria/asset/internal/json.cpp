#include "json.hpp"
namespace tria::asset::internal {

auto parseJson(const std::vector<char>& raw) noexcept -> JsonParseResult {
  // Verify that the input is sufficiently padded.
  assert(raw.capacity() - raw.size() >= simdjson::SIMDJSON_PADDING);

  thread_local static simdjson::dom::parser parser;
  return parser.parse(raw.data(), raw.size(), false);
}

} // namespace tria::asset::internal

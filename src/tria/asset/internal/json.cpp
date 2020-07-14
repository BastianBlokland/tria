#include "json.hpp"

namespace tria::asset::internal {

auto parseJson(const std::vector<char>& raw) noexcept -> JsonParseResult {
  thread_local static simdjson::dom::parser parser;
  return parser.parse(raw.data(), raw.size(), true);
}

} // namespace tria::asset::internal

#include "catch2/catch.hpp"
#include "tria/log/param.hpp"
#include <string>

namespace tria::log::tests {

namespace {

auto paramValueToString(const Param& param) {
  auto str = std::string{};
  param.writeValue(&str, true);
  return str;
}

} // namespace

TEST_CASE("Log write param", "[log]") {
  CHECK(paramValueToString({"key", 42}) == "42");
  CHECK(paramValueToString({"key", 100'000'000}) == "100000000");
  CHECK_THAT(paramValueToString({"key", 1.337F}), Catch::StartsWith("1.337"));
  CHECK_THAT(paramValueToString({"key", 1.333337}), Catch::StartsWith("1.333337"));
  CHECK(paramValueToString({"key", "Hello World"}) == "\"Hello World\"");
  CHECK(paramValueToString({"key", std::string{"Hello World"}}) == "\"Hello World\"");
}

} // namespace tria::log::tests

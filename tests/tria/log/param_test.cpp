#include "catch2/catch.hpp"
#include "tria/log/param.hpp"
#include <string>

using namespace std::literals;

namespace tria::log::tests {

namespace {

auto toStringPretty(const Param& param) {
  auto str = std::string{};
  param.writeValue(&str, Param::WriteMode::Pretty);
  return str;
}

auto toStringJson(const Param& param) {
  auto str = std::string{};
  param.writeValue(&str, Param::WriteMode::Json);
  return str;
}

} // namespace

TEST_CASE("Log write param", "[log]") {

  SECTION("Pretty") {
    CHECK(toStringPretty({"key", 42}) == "42");
    CHECK(toStringPretty({"key", 100'000'000}) == "100000000");

    CHECK_THAT(toStringPretty({"key", 1.337F}), Catch::StartsWith("1.337"));
    CHECK_THAT(toStringPretty({"key", 1.333337}), Catch::StartsWith("1.333337"));

    CHECK(toStringPretty({"key", "Hello World"}) == "Hello World");
    CHECK(toStringPretty({"key", std::string{"Hello World"}}) == "Hello World");
    CHECK(toStringPretty({"key", std::string{"Hello\tWorld\n"}}) == "Hello\\tWorld\\n");

    CHECK(toStringPretty({"key", 137ns}) == "137 ns");
    CHECK(toStringPretty({"key", 1337ns}) == "1.3 us");
    CHECK(toStringPretty({"key", 42ns}) == "42 ns");
    CHECK(toStringPretty({"key", 42us}) == "42 us");
    CHECK(toStringPretty({"key", 42ms}) == "42 ms");
    CHECK(toStringPretty({"key", 42s}) == "42 sec");
    CHECK(toStringPretty({"key", 420s}) == "420 sec");
    CHECK(toStringPretty({"key", 42us + 51ns}) == "42.1 us");
    CHECK(toStringPretty({"key", 42us + 49ns}) == "42 us");
    CHECK(toStringPretty({"key", 1s + 900ms}) == "1.9 sec");

    CHECK(toStringPretty({"key", MemSize{0}}) == "0 B");
    CHECK(toStringPretty({"key", MemSize{1024}}) == "1 KiB");
    CHECK(toStringPretty({"key", MemSize{1024UL * 1024}}) == "1 MiB");
    CHECK(toStringPretty({"key", MemSize{1024UL * 1024 * 1024}}) == "1 GiB");
    CHECK(toStringPretty({"key", MemSize{1024UL * 1024 * 1024 * 1024}}) == "1 TiB");
    CHECK(toStringPretty({"key", MemSize{1024UL * 1024 * 1024 * 1024 * 1024}}) == "1 PiB");
    CHECK(
        toStringPretty({"key", MemSize{1024UL * 1024 * 1024 * 1024 * 1024 * 1024}}) == "1024 PiB");

    CHECK(toStringPretty({"key", MemSize{42}}) == "42 B");
    CHECK(toStringPretty({"key", MemSize{4242}}) == "4.1 KiB");
    CHECK(toStringPretty({"key", MemSize{424242}}) == "414.3 KiB");
  }

  SECTION("Json") {
    CHECK(toStringJson({"key", 42}) == "42");
    CHECK(toStringJson({"key", 100'000'000}) == "100000000");

    CHECK_THAT(toStringJson({"key", 1.337F}), Catch::StartsWith("1.337"));
    CHECK_THAT(toStringJson({"key", 1.333337}), Catch::StartsWith("1.333337"));

    CHECK(toStringJson({"key", "Hello World"}) == "\"Hello World\"");
    CHECK(toStringJson({"key", std::string{"Hello World"}}) == "\"Hello World\"");
    CHECK(toStringJson({"key", std::string{"Hello\tWorld\n"}}) == "\"Hello\\tWorld\\n\"");

    CHECK(toStringJson({"key", 42ns}) == "42");
    CHECK(toStringJson({"key", 42us}) == "42000");
    CHECK(toStringJson({"key", 42ms}) == "42000000");
    CHECK(toStringJson({"key", 42s}) == "42000000000");
    CHECK(toStringJson({"key", 42s + 42ms + 42us + 42ns}) == "42042042042");

    CHECK(toStringJson({"key", MemSize{1024UL * 1024}}) == "1048576");
  }
}

} // namespace tria::log::tests

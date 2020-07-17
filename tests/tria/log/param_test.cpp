#include "catch2/catch.hpp"
#include "tria/log/param.hpp"
#include "tria/math/vec.hpp"
#include <ctime>
#include <string>

#if defined(_WIN32)
#define timezone _timezone
#endif

namespace tria::log {

namespace {

auto toStringPretty(const Param& param) {
  auto str = std::string{};
  param.writeValue(&str, ParamWriteMode::Pretty);
  return str;
}

auto toStringJson(const Param& param) {
  auto str = std::string{};
  param.writeValue(&str, ParamWriteMode::Json);
  return str;
}

auto getRefTimeT(int year, int month, int day, int hour, int min, int sec) -> TimePoint {
  tm t      = {};
  t.tm_year = year - 1900;
  t.tm_mon  = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min  = min;
  t.tm_sec  = sec;
  return std::chrono::system_clock::from_time_t(std::mktime(&t) - timezone);
}

/* Type to test logging custom types.
 */
struct CustomType1 final {
  int field;
};

/* Type to test logging custom types.
 */
struct CustomType2 final {
  int field;
};

} // namespace

/* Specialization of a value-factory for our CustomType1.
 */
template <>
struct ValueFactory<CustomType1> {
  auto operator()(CustomType1 t) const noexcept -> Value { return {t.field}; }
};

/* Specialization of a value-factory for our CustomType2.
 */
template <>
struct ValueFactory<CustomType2> {
  auto operator()(CustomType2 t) const noexcept -> std::vector<Value> {
    return {t.field, 42, "Hello World"};
  }
};

namespace tests {

TEST_CASE("[log] - Log parameters", "[log]") {

  using namespace std::chrono;
  using namespace std::literals;

  SECTION("Parameters can be pretty printed") {
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

    CHECK(
        toStringPretty({"key", getRefTimeT(2020, 7, 13, 12, 36, 42)}) ==
        "2020-07-13T12:36:42.000000Z");
    CHECK(
        toStringPretty({"key", getRefTimeT(2020, 7, 13, 12, 36, 42) + 1337ms}) ==
        "2020-07-13T12:36:43.337000Z");
    CHECK(
        toStringPretty({"key", getRefTimeT(2020, 7, 13, 12, 36, 42) + 42us}) ==
        "2020-07-13T12:36:42.000042Z");

    CHECK(toStringPretty({"key", MemSize{0}}) == "0 B");
    CHECK(toStringPretty({"key", MemSize{1024}}) == "1 KiB");
    CHECK(toStringPretty({"key", MemSize{1024ULL * 1024}}) == "1 MiB");
    CHECK(toStringPretty({"key", MemSize{1024ULL * 1024 * 1024}}) == "1 GiB");
    CHECK(toStringPretty({"key", MemSize{1024ULL * 1024 * 1024 * 1024}}) == "1 TiB");
    CHECK(toStringPretty({"key", MemSize{1024ULL * 1024 * 1024 * 1024 * 1024}}) == "1 PiB");
    CHECK(
        toStringPretty({"key", MemSize{1024ULL * 1024 * 1024 * 1024 * 1024 * 1024}}) == "1024 PiB");

    CHECK(toStringPretty({"key", MemSize{42}}) == "42 B");
    CHECK(toStringPretty({"key", MemSize{4242}}) == "4.1 KiB");
    CHECK(toStringPretty({"key", MemSize{424242}}) == "414.3 KiB");

    CHECK(toStringPretty({"key", math::color::red()}) == "1, 0, 0, 1");
    CHECK(toStringPretty({"key", math::Vec2i{42, 1337}}) == "42, 1337");

    const auto vec3 = math::Vec3i{1, 2, 3};
    CHECK(toStringPretty({"key", vec3}) == "1, 2, 3");

    CHECK(toStringPretty({"key", CustomType1{1337}}) == "1337");
    const auto ct1 = CustomType1{1337};
    CHECK(toStringPretty({"key", ct1}) == "1337");

    CHECK(toStringPretty({"key", CustomType2{1337}}) == "1337, 42, Hello World");
    const auto ct2 = CustomType2{1337};
    CHECK(toStringPretty({"key", ct2}) == "1337, 42, Hello World");

    CHECK(toStringPretty({"key", 1, 2, 3}) == "1, 2, 3");
    CHECK(
        toStringPretty({"key", 1, MemSize{424242}, 42ms, "Hello World"}) ==
        "1, 414.3 KiB, 42 ms, Hello World");
  }

  SECTION("Parameters can be json printed") {
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

    CHECK(
        toStringJson({"key", getRefTimeT(2020, 7, 13, 12, 36, 42)}) ==
        "\"2020-07-13T12:36:42.000000Z\"");

    CHECK(toStringJson({"key", MemSize{1024UL * 1024}}) == "1048576");

    CHECK(toStringJson({"key", math::color::red()}) == "[1, 0, 0, 1]");
    CHECK(toStringJson({"key", math::Vec2i{42, 1337}}) == "[42, 1337]");

    CHECK(toStringJson({"key", CustomType1{1337}}) == "1337");
    CHECK(toStringJson({"key", CustomType2{1337}}) == "[1337, 42, \"Hello World\"]");

    CHECK(toStringJson({"key", 1, 2, 3}) == "[1, 2, 3]");
    CHECK(
        toStringJson({"key", 1, MemSize{4242}, 42us, "Hello World"}) ==
        "[1, 4242, 42000, \"Hello World\"]");
  }
}

} // namespace tests

} // namespace tria::log

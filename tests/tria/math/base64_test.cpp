#include "catch2/catch.hpp"
#include "tria/math/base64.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Base64", "[math]") {

  SECTION("Decode 'Hello World'") {
    auto result = base64Decode("SGVsbG8gV29ybGQ=");
    CHECK(std::string(result.begin(), result.end()) == "Hello World");
  }

  SECTION("Input with missing padding decodes to same result") {
    auto result = base64Decode("SGVsbG8gV29ybGQ");
    CHECK(std::string(result.begin(), result.end()) == "Hello World");
  }

  SECTION("Decoding stops when an invalid base64 character is found") {
    auto result = base64Decode("SGVsbG8-gV29ybGQ");
    CHECK(std::string(result.begin(), result.end()) == "Hello");
  }

  SECTION("Decoding empty string results in empty output") {
    auto result = base64Decode("");
    CHECK(result.empty());
  }

  SECTION("Decode wikipedia example") {
    auto result =
        base64Decode("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                     "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                     "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                     "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                     "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=");
    CHECK(
        std::string(result.begin(), result.end()) ==
        "Man is distinguished, not only by his reason, but by this singular passion from other "
        "animals, which is a lust of the mind, that by a perseverance of delight in the continued "
        "and indefatigable generation of knowledge, exceeds the short vehemence of any carnal "
        "pleasure.");
  }

  SECTION("Decode input with 2 padding characters") {
    auto result = base64Decode("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
    CHECK(std::string(result.begin(), result.end()) == "any carnal pleasure");
  }
}

} // namespace tria::math::tests

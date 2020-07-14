#include "catch2/catch.hpp"
#include "tria/pal/utils.hpp"

namespace tria::pal::tests {

TEST_CASE("[pal] - Platform utils", "[pal]") {

  SECTION("Path to executable ends with the executable name") {
    CHECK_THAT(
        getCurExecutablePath().string(),
        (Catch::EndsWith("tria_tests") || Catch::EndsWith("tria_tests.exe")));
  }

  SECTION("Executable name can be retrieved") { CHECK(getCurExecutableName() == "tria_tests"); }

  SECTION("Thread name can be retrieved after successfully naming it") {
    auto name = std::string_view{"test_thread"};
    if (setThreadName(name)) {
      CHECK(getThreadName() == name);
    }
  }
}

} // namespace tria::pal::tests

#include "catch2/catch.hpp"
#include "tria/pal/utils.hpp"

namespace tria::pal::tests {

TEST_CASE("Platform utils", "[pal]") {

  SECTION("Executable path") {
    CHECK_THAT(
        getCurExecutablePath().string(),
        (Catch::EndsWith("tria_tests") || Catch::EndsWith("tria_tests.exe")));
  }

  SECTION("Naming threads") {
    auto name = std::string_view{"test_thread"};
    if (setThreadName(name)) {
      CHECK(getThreadName() == name);
    }
  }
}

} // namespace tria::pal::tests

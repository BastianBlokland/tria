#include "catch2/catch.hpp"
#include "tria/log/level.hpp"

namespace tria::log::tests {

TEST_CASE("Log level", "[log]") {

  SECTION("Level names") {
    CHECK(getName(Level::Debug) == std::string{"debug"});
    CHECK(getName(Level::Info) == std::string{"info"});
    CHECK(getName(Level::Warn) == std::string{"warn"});
    CHECK(getName(Level::Error) == std::string{"error"});
  }

  SECTION("Level mask") {
    const auto mask = Level::Error | Level::Debug;
    CHECK(isInMask(mask, Level::Error));
    CHECK(isInMask(mask, Level::Debug));
    CHECK(!isInMask(mask, Level::Info));
    CHECK(!isInMask(mask, Level::Warn));
  }
}

} // namespace tria::log::tests

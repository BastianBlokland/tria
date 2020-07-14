#include "catch2/catch.hpp"
#include "tria/log/level.hpp"

namespace tria::log::tests {

TEST_CASE("[log] - Log level", "[log]") {

  SECTION("Log levels have names") {
    CHECK(getName(Level::Debug) == std::string{"dbg"});
    CHECK(getName(Level::Info) == std::string{"inf"});
    CHECK(getName(Level::Warn) == std::string{"wrn"});
    CHECK(getName(Level::Error) == std::string{"err"});
  }

  SECTION("Level mask can be used to filter levels") {
    const auto mask = Level::Error | Level::Debug;
    CHECK(isInMask(mask, Level::Error));
    CHECK(isInMask(mask, Level::Debug));
    CHECK(!isInMask(mask, Level::Info));
    CHECK(!isInMask(mask, Level::Warn));
  }
}

} // namespace tria::log::tests

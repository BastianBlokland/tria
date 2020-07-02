#include "catch2/catch.hpp"
#include "mock_sink.hpp"
#include "tria/log/api.hpp"
#include <thread>

namespace tria::log::tests {

TEST_CASE("Logger", "[log]") {

  SECTION("Message publishing") {
    auto output = std::vector<Message>{};
    {
      auto logger    = Logger{makeMockSink(&output)};
      auto dynString = std::string{"dyn_string"};
      LOG_I(
          &logger,
          "test_message",
          {"param1", 42},
          {"param2", 1337.42},
          {"param3", "static_string"},
          {"param4", dynString});
    }

    REQUIRE(output.size() == 1);
    const auto& msg = output[0];

    // Check that meta-data is correct.
    CHECK(msg.getMeta()->getTxt() == std::string{"test_message"});
    CHECK_THAT(std::string{msg.getMeta()->getFile()}, Catch::EndsWith("logger_test.cpp"));
    CHECK(msg.getMeta()->getLevel() == Level::Info);

    // Check that parameters are correct.
    CHECK_THAT(
        std::vector<Param>(msg.begin(), msg.end()),
        Catch::UnorderedEquals(std::vector<Param>{{"param1", 42},
                                                  {"param2", 1337.42},
                                                  {"param3", "static_string"},
                                                  {"param4", std::string{"dyn_string"}}}));
  }

  SECTION("Multithreaded publishing") {
    constexpr static int numThreads          = 5;
    constexpr static int numMessagePerThread = 10'000;

    auto output = std::vector<Message>{};
    {
      auto logger = Logger{makeMockSink(&output)};

      // Post messages from multiple threads.
      auto threads = std::vector<std::thread>{};
      for (auto threadNum = 0U; threadNum != numThreads; ++threadNum) {
        threads.push_back(std::thread{[threadNum, &logger]() {
          for (auto msgNum = 0U; msgNum != numMessagePerThread; ++msgNum) {
            LOG_I(&logger, "test_message", {"threadNum", threadNum}, {"msgNum", msgNum});
          }
        }});
      }

      // Wait for all threads to finish.
      for (auto& thread : threads) {
        thread.join();
      }
    }

    CHECK(output.size() == numThreads * numMessagePerThread);
  }
}

} // namespace tria::log::tests

#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/err/asset_type_err.hpp"
#include "tria/asset/shader.hpp"
#include "utils.hpp"
#include <random>
#include <thread>
#include <tuple>

namespace tria::asset::tests {

TEST_CASE("[asset] - Database", "[asset]") {

  SECTION("Loading asset yields its contents") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.tst", "Hello World");

      auto db = Database{nullptr, dir};
      CHECK_RAW_ASSET(db.get("test.tst"), "Hello World");

      // Load the same file again, content should be the same.
      CHECK_RAW_ASSET(db.get("test.tst"), "Hello World");
    });
  }

  SECTION("Downcasting to an incorrect type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.tst", "Hello World");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.tst")->downcast<Shader>(), err::AssetTypeErr);
    });
  }

  SECTION("Loading non-existing asset throws") {
    withTempDir([](const fs::path& dir) {
      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("nothing.txt"), err::AssetLoadErr);
    });
  }

  SECTION("Assets can be loaded in parallel") {
    constexpr static int numFiles          = 100;
    constexpr static int numThreads        = 10;
    constexpr static int numLoadsPerThread = 1'000;

    withTempDir([](const fs::path& dir) {
      auto db = Database{nullptr, dir};

      // Generate test files.
      auto files = std::vector<std::tuple<std::string, std::string>>();
      for (auto fileNum = 0U; fileNum != numFiles; ++fileNum) {
        const auto fileNumStr  = std::to_string(fileNum);
        const auto filePath    = fileNumStr + ".tst";
        const auto fileContent = std::string{"Hello "} + fileNumStr;
        writeFile(dir / filePath, fileContent);
        files.push_back({filePath, fileContent});
      }

      // Load random files in parallel.
      auto threads = std::vector<std::thread>{};
      for (auto threadNum = 0U; threadNum != numThreads; ++threadNum) {
        threads.push_back(std::thread{[&files, &db]() {
          auto rngDev  = std::random_device{};
          auto rngGen  = std::mt19937{rngDev()};
          auto rngDist = std::uniform_int_distribution<size_t>{0, files.size() - 1};

          for (auto loadNum = 0U; loadNum != numLoadsPerThread; ++loadNum) {
            // Pick a random file and load it.
            const auto& [id, content] = files[rngDist(rngGen)];
            CHECK_RAW_ASSET(db.get(id), content);
          }
        }});
      }

      // Wait for all threads to finish.
      for (auto& thread : threads) {
        thread.join();
      }
    });
  }
}

} // namespace tria::asset::tests

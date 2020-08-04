#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/shader.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Shader", "[asset]") {

  SECTION("Spir-v vertex shaders are identified based on their name") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", "");

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.vert.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.vert.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvVertex);
    });
  }

  SECTION("Spir-v fragment shaders are identified based on their name") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.frag.spv", "");

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.frag.spv")->getKind() == AssetKind::Shader);
      CHECK(
          db.get("test.frag.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvFragment);
    });
  }

  SECTION("Loading spir-v shader with unexpected name throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.spv", "");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::AssetLoadErr);
    });
  }

  SECTION("Loading spir-v shader yields its contents") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", "Hello Shader");

      auto db      = Database{nullptr, dir};
      auto* shader = db.get("test.vert.spv")->downcast<Shader>();
      CHECK(
          std::string(reinterpret_cast<const char*>(shader->getBegin()), shader->getSize()) ==
          "Hello Shader");
    });
  }
}

} // namespace tria::asset::tests

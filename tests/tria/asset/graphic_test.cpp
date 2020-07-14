#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/graphic.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Graphic", "[asset]") {

  SECTION("Shaders are loaded as part of the graphic") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", "");
      writeFile(dir / "test.frag.spv", "");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"vertShader\": \"test.vert.spv\","
          "\"fragShader\": \"test.frag.spv\""
          "}");

      auto db   = Database{nullptr, dir};
      auto* gfx = db.get("test.gfx")->downcast<Graphic>();
      CHECK(gfx->getVertShader()->getShaderKind() == ShaderKind::SpvVertex);
      CHECK(gfx->getFragShader()->getShaderKind() == ShaderKind::SpvFragment);
    });
  }

  SECTION("Loading a graphic with invalid json throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.gfx",
          "{"
          "\"vertShader\": \"test.vert.spv\","
          "\"fragShader\": \"test.frag.spv\"");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::AssetLoadErr);
    });
  }

  SECTION("Loading a graphic without a vertex shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.frag.spv", "");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"fragShader\": \"test.frag.spv\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::AssetLoadErr);
    });
  }

  SECTION("Loading a graphic without a fragment shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", "");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"vertShader\": \"test.vert.spv\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::AssetLoadErr);
    });
  }

  SECTION("Loading a graphic with incorrect vertex shader type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.frag.spv", "");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"vertShader\": \"test.frag.spv\","
          "\"fragShader\": \"test.frag.spv\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::AssetLoadErr);
    });
  }

  SECTION("Loading a graphic with incorrect fragment shader type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", "");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"vertShader\": \"test.vert.spv\","
          "\"fragShader\": \"test.vert.spv\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::AssetLoadErr);
    });
  }
}

} // namespace tria::asset::tests

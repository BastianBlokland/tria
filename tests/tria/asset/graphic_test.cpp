#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/graphic_err.hpp"
#include "tria/asset/err/json_err.hpp"
#include "tria/asset/graphic.hpp"
#include "tria/math/base64.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

namespace {

auto getTestVertShader() noexcept {
  // Dummy vertex shader compiled to spir-v.
  return math::base64Decode("AwIjBwAAAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                            "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                            "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA=");
}

auto getTestFragShader() noexcept {
  // Dummy fragment shader compiled to spir-v.
  return math::base64Decode(
      "AwIjBwAAAQAIAA0ADAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8ABgAE"
      "AAAABAAAAG1haW4AAAAACQAAABAAAwAEAAAABwAAAEcABAAJAAAAHgAAAAAAAAATAAIAAgAAACEAAwADAAAAAgAAABYA"
      "AwAGAAAAIAAAABcABAAHAAAABgAAAAQAAAAgAAQACAAAAAMAAAAHAAAAOwAEAAgAAAAJAAAAAwAAACsABAAGAAAACgAA"
      "AAAAgD8sAAcABwAAAAsAAAAKAAAACgAAAAoAAAAKAAAANgAFAAIAAAAEAAAAAAAAAAMAAAD4AAIABQAAAD4AAwAJAAAA"
      "CwAAAP0AAQA4AAEA");
}

} // namespace

TEST_CASE("[asset] - Graphic", "[asset]") {

  SECTION("Shaders and mesh are loaded as part of the graphic") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", getTestVertShader());
      writeFile(dir / "test.frag.spv", getTestFragShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\", \"test.frag.spv\"],"
          "\"mesh\": \"test.obj\","
          "\"blend\": \"alpha\","
          "\"depthTest\": \"less\""
          "}");

      auto db   = Database{nullptr, dir};
      auto* gfx = db.get("test.gfx")->downcast<Graphic>();
      REQUIRE(gfx->getShaderCount() == 2);
      CHECK(gfx->getShadersBegin()[0]->getShaderKind() == ShaderKind::SpvVertex);
      CHECK(gfx->getShadersBegin()[1]->getShaderKind() == ShaderKind::SpvFragment);
      CHECK(gfx->getMesh()->getKind() == AssetKind::Mesh);
      CHECK(gfx->getBlendMode() == BlendMode::Alpha);
      CHECK(gfx->getDepthTestMode() == DepthTestMode::Less);
    });
  }

  SECTION("Texture samplers are optionally loaded as part of the graphic") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", getTestVertShader());
      writeFile(dir / "test.frag.spv", getTestFragShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(dir / "test.ppm", "P3 1 1 255 1 42 137");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\", \"test.frag.spv\"],"
          "\"mesh\": \"test.obj\","
          "\"samplers\": [{ \"texture\": \"test.ppm\", \"filter\": \"nearest\", \"anisotropy\": "
          "\"x4\"}]"
          "}");

      auto db   = Database{nullptr, dir};
      auto* gfx = db.get("test.gfx")->downcast<Graphic>();
      REQUIRE(gfx->getSamplerCount() == 1);
      CHECK(*gfx->getSamplerBegin()->getTexture()->getPixelBegin() == Pixel{1, 42, 137, 255});
      CHECK(gfx->getSamplerBegin()->getFilterMode() == FilterMode::Nearest);
      CHECK(gfx->getSamplerBegin()->getAnisoMode() == AnisotropyMode::X4);
    });
  }

  SECTION("Loading a graphic with invalid json throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\",\"test.frag.spv\"]");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::JsonErr);
    });
  }

  SECTION("Loading a graphic without a vertex shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.frag.spv", getTestFragShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.frag.spv\"],"
          "\"mesh\": \"test.obj\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::GraphicErr);
    });
  }

  SECTION("Loading a graphic without a fragment shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", getTestVertShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\"],"
          "\"mesh\": \"test.obj\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::GraphicErr);
    });
  }

  SECTION("Loading a graphic without a mesh throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", getTestVertShader());
      writeFile(dir / "test.frag.spv", getTestFragShader());
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\",\"test.frag.spv\"]"
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::GraphicErr);
    });
  }

  SECTION("Loading a graphic with incorrect vertex shader type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.frag.spv", getTestFragShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.frag.spv\", \"test.frag.spv\"],"
          "\"mesh\": \"test.obj\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::GraphicErr);
    });
  }

  SECTION("Loading a graphic with incorrect fragment shader type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.vert.spv", getTestVertShader());
      writeFile(dir / "test.obj", "v 0.0 0.0 0.0\nf 1 1 1\n");
      writeFile(
          dir / "test.gfx",
          "{"
          "\"shaders\": [\"test.vert.spv\", \"test.vert.spv\"],"
          "\"mesh\": \"test.obj\""
          "}");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.gfx"), err::GraphicErr);
    });
  }
}

} // namespace tria::asset::tests

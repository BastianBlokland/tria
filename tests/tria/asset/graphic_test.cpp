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
  // Dummy vertex shader compiled to spir-v 1.3.
  return math::base64Decode("AwIjBwADAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA"
                            "4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAAA2"
                            "AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA=");
}

auto getTestFragShader() noexcept {
  // Dummy fragment shader compiled to spir-v 1.3.
  return math::base64Decode(
      "AwIjBwADAQAIAA0ADAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8ABgAE"
      "AAAABAAAAG1haW4AAAAACQAAABAAAwAEAAAABwAAAAMAAwACAAAAwgEAAAQACQBHTF9BUkJfc2VwYXJhdGVfc2hhZGVy"
      "X29iamVjdHMAAAQACgBHTF9HT09HTEVfY3BwX3N0eWxlX2xpbmVfZGlyZWN0aXZlAAAEAAgAR0xfR09PR0xFX2luY2x1"
      "ZGVfZGlyZWN0aXZlAAUABAAEAAAAbWFpbgAAAAAFAAUACQAAAG91dENvbG9yAAAAAEcABAAJAAAAHgAAAAAAAAATAAIA"
      "AgAAACEAAwADAAAAAgAAABYAAwAGAAAAIAAAABcABAAHAAAABgAAAAQAAAAgAAQACAAAAAMAAAAHAAAAOwAEAAgAAAAJ"
      "AAAAAwAAACsABAAGAAAACgAAAAAAgD8sAAcABwAAAAsAAAAKAAAACgAAAAoAAAAKAAAANgAFAAIAAAAEAAAAAAAAAAMA"
      "AAD4AAIABQAAAD4AAwAJAAAACwAAAP0AAQA4AAEA");
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
          "\"topology\": \"lines\","
          "\"rasterizer\": \"lines\","
          "\"blend\": \"alpha\","
          "\"depthTest\": \"less\","
          "\"cull\": \"front\""
          "}");

      auto db   = Database{nullptr, dir};
      auto* gfx = db.get("test.gfx")->downcast<Graphic>();
      REQUIRE(gfx->getShaderCount() == 2);
      CHECK(gfx->getShaderBegin()[0]->getShaderKind() == ShaderKind::SpvVertex);
      CHECK(gfx->getShaderBegin()[1]->getShaderKind() == ShaderKind::SpvFragment);
      CHECK(gfx->getVertexTopology() == VertexTopology::Lines);
      CHECK(gfx->getRasterizerMode() == RasterizerMode::Lines);
      CHECK(gfx->getMesh()->getKind() == AssetKind::Mesh);
      CHECK(gfx->getBlendMode() == BlendMode::Alpha);
      CHECK(gfx->getDepthTestMode() == DepthTestMode::Less);
      CHECK(gfx->getCullMode() == CullMode::Front);
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

#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/shader.hpp"
#include "tria/math/base64.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Shader SpirV", "[asset]") {

  SECTION("Spir-v vertex shaders are identified") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.spv",
          math::base64Decode("AwIjBwAAAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA="));

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvVertex);
    });
  }

  SECTION("Spir-v fragment shaders are identified") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.spv",
          math::base64Decode(
              "AwIjBwAAAQAIAA0ADAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAA"
              "AA8ABgAEAAAABAAAAG1haW4AAAAACQAAABAAAwAEAAAABwAAAEcABAAJAAAAHgAAAAAAAAATAAIAAgAAACEA"
              "AwADAAAAAgAAABYAAwAGAAAAIAAAABcABAAHAAAABgAAAAQAAAAgAAQACAAAAAMAAAAHAAAAOwAEAAgAAAAJ"
              "AAAAAwAAACsABAAGAAAACgAAAAAAgD8sAAcABwAAAAsAAAAKAAAACgAAAAoAAAAKAAAANgAFAAIAAAAEAAAA"
              "AAAAAAMAAAD4AAIABQAAAD4AAwAJAAAACwAAAP0AAQA4AAEA"));

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvFragment);
    });
  }

  SECTION("Loading malformed spir-v shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.spv", "Hello world");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::AssetLoadErr);
    });
  }

  SECTION("Loading spir-v shader yields its contents") {
    withTempDir([](const fs::path& dir) {
      const auto rawContent =
          math::base64Decode("AwIjBwAAAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA=");
      writeFile(dir / "test.spv", rawContent);

      auto db      = Database{nullptr, dir};
      auto* shader = db.get("test.spv")->downcast<Shader>();
      REQUIRE(shader->getSize() == rawContent.size());
      CHECK(std::memcmp(shader->getBegin(), rawContent.begin(), rawContent.size()) == 0);
    });
  }
}

} // namespace tria::asset::tests

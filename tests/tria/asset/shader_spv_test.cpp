#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/shader_spv_err.hpp"
#include "tria/asset/shader.hpp"
#include "tria/math/base64.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Shader SpirV", "[asset]") {

  SECTION("Spir-v vertex shaders are identified") {
    withTempDir([](const fs::path& dir) {
      // Dummy vertex shader compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode("AwIjBwADAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA="));

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvVertex);
    });
  }

  SECTION("Spir-v fragment shaders are identified") {
    withTempDir([](const fs::path& dir) {
      // Dummy fragment shader compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode(
              "AwIjBwADAQAIAA0ADAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAA"
              "AA8ABgAEAAAABAAAAG1haW4AAAAACQAAABAAAwAEAAAABwAAAAMAAwACAAAAwgEAAAQACQBHTF9BUkJfc2Vw"
              "YXJhdGVfc2hhZGVyX29iamVjdHMAAAQACgBHTF9HT09HTEVfY3BwX3N0eWxlX2xpbmVfZGlyZWN0aXZlAAAE"
              "AAgAR0xfR09PR0xFX2luY2x1ZGVfZGlyZWN0aXZlAAUABAAEAAAAbWFpbgAAAAAFAAUACQAAAG91dENvbG9y"
              "AAAAAEcABAAJAAAAHgAAAAAAAAATAAIAAgAAACEAAwADAAAAAgAAABYAAwAGAAAAIAAAABcABAAHAAAABgAA"
              "AAQAAAAgAAQACAAAAAMAAAAHAAAAOwAEAAgAAAAJAAAAAwAAACsABAAGAAAACgAAAAAAgD8sAAcABwAAAAsA"
              "AAAKAAAACgAAAAoAAAAKAAAANgAFAAIAAAAEAAAAAAAAAAMAAAD4AAIABQAAAD4AAwAJAAAACwAAAP0AAQA4"
              "AAEA"));

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.spv")->downcast<Shader>()->getShaderKind() == ShaderKind::SpvFragment);
    });
  }

  SECTION("Spir-v entry-point name is read") {
    withTempDir([](const fs::path& dir) {
      // Dummy fragment shader compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode("AwIjBwADAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA="));

      auto db = Database{nullptr, dir};
      REQUIRE(db.get("test.spv")->getKind() == AssetKind::Shader);
      CHECK(db.get("test.spv")->downcast<Shader>()->getEntryPointName() == "main");
    });
  }

  SECTION("Texture resources are detected") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with 6 texture inputs compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode(
              "AwIjBwADAQAIAA0AEAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAA"
              "AA8ABQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2Jq"
              "ZWN0cwAABAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5j"
              "bHVkZV9kaXJlY3RpdmUABQAEAAQAAABtYWluAAAAAAUABAAKAAAAdGV4MQAAAAAFAAQACwAAAHRleDIAAAAA"
              "BQAEAAwAAAB0ZXgzAAAAAAUABAANAAAAdGV4NAAAAAAFAAQADgAAAHRleDUAAAAABQAEAA8AAAB0ZXg2AAAA"
              "AEcABAAKAAAAIgAAAAAAAABHAAQACgAAACEAAAAAAAAARwAEAAsAAAAiAAAAAgAAAEcABAALAAAAIQAAAAAA"
              "AABHAAQADAAAACIAAAACAAAARwAEAAwAAAAhAAAAAQAAAEcABAANAAAAIgAAAAQAAABHAAQADQAAACEAAAAA"
              "AAAARwAEAA4AAAAiAAAABAAAAEcABAAOAAAAIQAAAAEAAABHAAQADwAAACIAAAAEAAAARwAEAA8AAAAhAAAA"
              "BwAAABMAAgACAAAAIQADAAMAAAACAAAAFgADAAYAAAAgAAAAGQAJAAcAAAAGAAAAAQAAAAAAAAAAAAAAAAAA"
              "AAEAAAAAAAAAGwADAAgAAAAHAAAAIAAEAAkAAAAAAAAACAAAADsABAAJAAAACgAAAAAAAAA7AAQACQAAAAsA"
              "AAAAAAAAOwAEAAkAAAAMAAAAAAAAADsABAAJAAAADQAAAAAAAAA7AAQACQAAAA4AAAAAAAAAOwAEAAkAAAAP"
              "AAAAAAAAADYABQACAAAABAAAAAAAAAADAAAA+AACAAUAAAD9AAEAOAABAA=="));

      auto db            = Database{nullptr, dir};
      const auto* shader = db.get("test.spv")->downcast<Shader>();
      REQUIRE(shader->getResourceCount() == 6);
      CHECK(
          std::vector<ShaderResource>(shader->getResourceBegin(), shader->getResourceEnd()) ==
          std::vector<ShaderResource>{
              {ShaderResourceKind::Texture, 0U, 0U},
              {ShaderResourceKind::Texture, 2U, 0U},
              {ShaderResourceKind::Texture, 2U, 1U},
              {ShaderResourceKind::Texture, 4U, 0U},
              {ShaderResourceKind::Texture, 4U, 1U},
              {ShaderResourceKind::Texture, 4U, 7U},
          });
    });
  }

  SECTION("Uniform buffer resources are detected") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with 6 storage buffer inputs compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode(
              "AwIjBwADAQAIAA0AIwAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAA"
              "AA8ABQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2Jq"
              "ZWN0cwAABAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5j"
              "bHVkZV9kaXJlY3RpdmUABQAEAAQAAABtYWluAAAAAAUABAAIAAAARGF0YQAAAAAGAAcACAAAAAAAAABtZWFu"
              "aW5nT2ZMaWZlAAAABQAFAAwAAABEYXRhQnVmZmVyMQAGAAUADAAAAAAAAABkYXRhAAAAAAUAAwAOAAAAZDEA"
              "AAUABQAQAAAARGF0YUJ1ZmZlcjIABgAFABAAAAAAAAAAZGF0YQAAAAAFAAMAEgAAAGQyAAAFAAUAFAAAAERh"
              "dGFCdWZmZXIzAAYABQAUAAAAAAAAAGRhdGEAAAAABQADABYAAABkMwAABQAFABgAAABEYXRhQnVmZmVyNAAG"
              "AAUAGAAAAAAAAABkYXRhAAAAAAUAAwAaAAAAZDQAAAUABQAcAAAARGF0YUJ1ZmZlcjUABgAFABwAAAAAAAAA"
              "ZGF0YQAAAAAFAAMAHgAAAGQ1AAAFAAUAIAAAAERhdGFCdWZmZXI2AAYABQAgAAAAAAAAAGRhdGEAAAAABQAD"
              "ACIAAABkNgAASAAFAAgAAAAAAAAAIwAAAAAAAABHAAQACwAAAAYAAAAQAAAASAAFAAwAAAAAAAAAIwAAAAAA"
              "AABHAAMADAAAAAIAAABHAAQADgAAACIAAAAAAAAARwAEAA4AAAAhAAAAAAAAAEcABAAPAAAABgAAABAAAABI"
              "AAUAEAAAAAAAAAAjAAAAAAAAAEcAAwAQAAAAAgAAAEcABAASAAAAIgAAAAIAAABHAAQAEgAAACEAAAAAAAAA"
              "RwAEABMAAAAGAAAAEAAAAEgABQAUAAAAAAAAACMAAAAAAAAARwADABQAAAACAAAARwAEABYAAAAiAAAAAgAA"
              "AEcABAAWAAAAIQAAAAEAAABHAAQAFwAAAAYAAAAQAAAASAAFABgAAAAAAAAAIwAAAAAAAABHAAMAGAAAAAIA"
              "AABHAAQAGgAAACIAAAAEAAAARwAEABoAAAAhAAAAAAAAAEcABAAbAAAABgAAABAAAABIAAUAHAAAAAAAAAAj"
              "AAAAAAAAAEcAAwAcAAAAAgAAAEcABAAeAAAAIgAAAAQAAABHAAQAHgAAACEAAAABAAAARwAEAB8AAAAGAAAA"
              "EAAAAEgABQAgAAAAAAAAACMAAAAAAAAARwADACAAAAACAAAARwAEACIAAAAiAAAABAAAAEcABAAiAAAAIQAA"
              "AAcAAAATAAIAAgAAACEAAwADAAAAAgAAABYAAwAGAAAAIAAAABcABAAHAAAABgAAAAQAAAAeAAMACAAAAAcA"
              "AAAVAAQACQAAACAAAAAAAAAAKwAEAAkAAAAKAAAAAQAAABwABAALAAAACAAAAAoAAAAeAAMADAAAAAsAAAAg"
              "AAQADQAAAAIAAAAMAAAAOwAEAA0AAAAOAAAAAgAAABwABAAPAAAACAAAAAoAAAAeAAMAEAAAAA8AAAAgAAQA"
              "EQAAAAIAAAAQAAAAOwAEABEAAAASAAAAAgAAABwABAATAAAACAAAAAoAAAAeAAMAFAAAABMAAAAgAAQAFQAA"
              "AAIAAAAUAAAAOwAEABUAAAAWAAAAAgAAABwABAAXAAAACAAAAAoAAAAeAAMAGAAAABcAAAAgAAQAGQAAAAIA"
              "AAAYAAAAOwAEABkAAAAaAAAAAgAAABwABAAbAAAACAAAAAoAAAAeAAMAHAAAABsAAAAgAAQAHQAAAAIAAAAc"
              "AAAAOwAEAB0AAAAeAAAAAgAAABwABAAfAAAACAAAAAoAAAAeAAMAIAAAAB8AAAAgAAQAIQAAAAIAAAAgAAAA"
              "OwAEACEAAAAiAAAAAgAAADYABQACAAAABAAAAAAAAAADAAAA+AACAAUAAAD9AAEAOAABAA=="));

      auto db            = Database{nullptr, dir};
      const auto* shader = db.get("test.spv")->downcast<Shader>();
      REQUIRE(shader->getResourceCount() == 6);
      CHECK(
          std::vector<ShaderResource>(shader->getResourceBegin(), shader->getResourceEnd()) ==
          std::vector<ShaderResource>{
              {ShaderResourceKind::UniformBuffer, 0U, 0U},
              {ShaderResourceKind::UniformBuffer, 2U, 0U},
              {ShaderResourceKind::UniformBuffer, 2U, 1U},
              {ShaderResourceKind::UniformBuffer, 4U, 0U},
              {ShaderResourceKind::UniformBuffer, 4U, 1U},
              {ShaderResourceKind::UniformBuffer, 4U, 7U},
          });
    });
  }

  SECTION("Storage buffer resources are detected") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with 6 storage buffer inputs compiled to spir-v 1.3.
      writeFile(
          dir / "test.spv",
          math::base64Decode(
              "AwIjBwADAQAIAA0AIQAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAA"
              "AA8ABQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2Jq"
              "ZWN0cwAABAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5j"
              "bHVkZV9kaXJlY3RpdmUABQAEAAQAAABtYWluAAAAAAUABAAIAAAARGF0YQAAAAAGAAcACAAAAAAAAABtZWFu"
              "aW5nT2ZMaWZlAAAABQAFAAoAAABEYXRhQnVmZmVyMQAGAAUACgAAAAAAAABkYXRhAAAAAAUAAwAMAAAAZDEA"
              "AAUABQAOAAAARGF0YUJ1ZmZlcjIABgAFAA4AAAAAAAAAZGF0YQAAAAAFAAMAEAAAAGQyAAAFAAUAEgAAAERh"
              "dGFCdWZmZXIzAAYABQASAAAAAAAAAGRhdGEAAAAABQADABQAAABkMwAABQAFABYAAABEYXRhQnVmZmVyNAAG"
              "AAUAFgAAAAAAAABkYXRhAAAAAAUAAwAYAAAAZDQAAAUABQAaAAAARGF0YUJ1ZmZlcjUABgAFABoAAAAAAAAA"
              "ZGF0YQAAAAAFAAMAHAAAAGQ1AAAFAAUAHgAAAERhdGFCdWZmZXI2AAYABQAeAAAAAAAAAGRhdGEAAAAABQAD"
              "ACAAAABkNgAASAAFAAgAAAAAAAAAIwAAAAAAAABHAAQACQAAAAYAAAAQAAAASAAEAAoAAAAAAAAAGAAAAEgA"
              "BQAKAAAAAAAAACMAAAAAAAAARwADAAoAAAACAAAARwAEAAwAAAAiAAAAAAAAAEcABAAMAAAAIQAAAAAAAABH"
              "AAQADQAAAAYAAAAQAAAASAAEAA4AAAAAAAAAGAAAAEgABQAOAAAAAAAAACMAAAAAAAAARwADAA4AAAACAAAA"
              "RwAEABAAAAAiAAAAAgAAAEcABAAQAAAAIQAAAAAAAABHAAQAEQAAAAYAAAAQAAAASAAEABIAAAAAAAAAGAAA"
              "AEgABQASAAAAAAAAACMAAAAAAAAARwADABIAAAACAAAARwAEABQAAAAiAAAAAgAAAEcABAAUAAAAIQAAAAEA"
              "AABHAAQAFQAAAAYAAAAQAAAASAAEABYAAAAAAAAAGAAAAEgABQAWAAAAAAAAACMAAAAAAAAARwADABYAAAAC"
              "AAAARwAEABgAAAAiAAAABAAAAEcABAAYAAAAIQAAAAAAAABHAAQAGQAAAAYAAAAQAAAASAAEABoAAAAAAAAA"
              "GAAAAEgABQAaAAAAAAAAACMAAAAAAAAARwADABoAAAACAAAARwAEABwAAAAiAAAABAAAAEcABAAcAAAAIQAA"
              "AAEAAABHAAQAHQAAAAYAAAAQAAAASAAEAB4AAAAAAAAAGAAAAEgABQAeAAAAAAAAACMAAAAAAAAARwADAB4A"
              "AAACAAAARwAEACAAAAAiAAAABAAAAEcABAAgAAAAIQAAAAcAAAATAAIAAgAAACEAAwADAAAAAgAAABYAAwAG"
              "AAAAIAAAABcABAAHAAAABgAAAAQAAAAeAAMACAAAAAcAAAAdAAMACQAAAAgAAAAeAAMACgAAAAkAAAAgAAQA"
              "CwAAAAwAAAAKAAAAOwAEAAsAAAAMAAAADAAAAB0AAwANAAAACAAAAB4AAwAOAAAADQAAACAABAAPAAAADAAA"
              "AA4AAAA7AAQADwAAABAAAAAMAAAAHQADABEAAAAIAAAAHgADABIAAAARAAAAIAAEABMAAAAMAAAAEgAAADsA"
              "BAATAAAAFAAAAAwAAAAdAAMAFQAAAAgAAAAeAAMAFgAAABUAAAAgAAQAFwAAAAwAAAAWAAAAOwAEABcAAAAY"
              "AAAADAAAAB0AAwAZAAAACAAAAB4AAwAaAAAAGQAAACAABAAbAAAADAAAABoAAAA7AAQAGwAAABwAAAAMAAAA"
              "HQADAB0AAAAIAAAAHgADAB4AAAAdAAAAIAAEAB8AAAAMAAAAHgAAADsABAAfAAAAIAAAAAwAAAA2AAUAAgAA"
              "AAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA="));

      auto db            = Database{nullptr, dir};
      const auto* shader = db.get("test.spv")->downcast<Shader>();
      REQUIRE(shader->getResourceCount() == 6);
      CHECK(
          std::vector<ShaderResource>(shader->getResourceBegin(), shader->getResourceEnd()) ==
          std::vector<ShaderResource>{
              {ShaderResourceKind::StorageBuffer, 0U, 0U},
              {ShaderResourceKind::StorageBuffer, 2U, 0U},
              {ShaderResourceKind::StorageBuffer, 2U, 1U},
              {ShaderResourceKind::StorageBuffer, 4U, 0U},
              {ShaderResourceKind::StorageBuffer, 4U, 1U},
              {ShaderResourceKind::StorageBuffer, 4U, 7U},
          });
    });
  }

  SECTION("Loading spir-v shader yields its contents") {
    withTempDir([](const fs::path& dir) {
      // Dummy fragment shader compiled to spir-v 1.3.
      const auto rawContent =
          math::base64Decode("AwIjBwADAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA=");
      writeFile(dir / "test.spv", rawContent);

      auto db      = Database{nullptr, dir};
      auto* shader = db.get("test.spv")->downcast<Shader>();
      REQUIRE(shader->getSize() == rawContent.size());
      CHECK(std::memcmp(shader->getBegin(), rawContent.begin(), rawContent.size()) == 0);
    });
  }

  SECTION("Loading unsupported spir-v version throws") {
    withTempDir([](const fs::path& dir) {
      // Dummy vertex shader compiled to spv 1.0.
      const auto rawContent =
          math::base64Decode("AwIjBwAAAQAIAA0ABgAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAA"
                             "A4AAwAAAAAAAQAAAA8ABQAAAAAABAAAAG1haW4AAAAAEwACAAIAAAAhAAMAAwAAAAIAAA"
                             "A2AAUAAgAAAAQAAAAAAAAAAwAAAPgAAgAFAAAA/QABADgAAQA=");
      writeFile(dir / "test.spv", rawContent);

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::ShaderSpvErr);
    });
  }

  SECTION("Loading shader where set exceeds maximum throws") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with one texture using set 33 and binding 1 compiled to spv 1.3.
      const auto rawContent = math::base64Decode(
          "AwIjBwADAQAIAA0ACwAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8A"
          "BQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2JqZWN0cwAA"
          "BAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5jbHVkZV9kaXJl"
          "Y3RpdmUABQAEAAQAAABtYWluAAAAAAUAAwAKAAAAdGV4AEcABAAKAAAAIgAAACEAAABHAAQACgAAACEAAAABAAAA"
          "EwACAAIAAAAhAAMAAwAAAAIAAAAWAAMABgAAACAAAAAZAAkABwAAAAYAAAABAAAAAAAAAAAAAAAAAAAAAQAAAAAA"
          "AAAbAAMACAAAAAcAAAAgAAQACQAAAAAAAAAIAAAAOwAEAAkAAAAKAAAAAAAAADYABQACAAAABAAAAAAAAAADAAAA"
          "+AACAAUAAAD9AAEAOAABAA==");
      writeFile(dir / "test.spv", rawContent);

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::ShaderSpvErr);
    });
  }

  SECTION("Loading shader where binding exceeds maximum throws") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with one texture using set 1 and binding 32 compiled to spv 1.3.
      const auto rawContent = math::base64Decode(
          "AwIjBwADAQAIAA0ACwAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8A"
          "BQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2JqZWN0cwAA"
          "BAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5jbHVkZV9kaXJl"
          "Y3RpdmUABQAEAAQAAABtYWluAAAAAAUAAwAKAAAAdGV4AEcABAAKAAAAIgAAAAEAAABHAAQACgAAACEAAAAgAAAA"
          "EwACAAIAAAAhAAMAAwAAAAIAAAAWAAMABgAAACAAAAAZAAkABwAAAAYAAAABAAAAAAAAAAAAAAAAAAAAAQAAAAAA"
          "AAAbAAMACAAAAAcAAAAgAAQACQAAAAAAAAAIAAAAOwAEAAkAAAAKAAAAAAAAADYABQACAAAABAAAAAAAAAADAAAA"
          "+AACAAUAAAD9AAEAOAABAA==");
      writeFile(dir / "test.spv", rawContent);

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::ShaderSpvErr);
    });
  }

  SECTION("Loading shader with multiple variables using the same set and binding throws") {
    withTempDir([](const fs::path& dir) {
      // Vertex shader with two texture using the same set and binding compiled to spv 1.3.
      const auto rawContent = math::base64Decode(
          "AwIjBwADAQAIAA0ADAAAAAAAAAARAAIAAQAAAAsABgABAAAAR0xTTC5zdGQuNDUwAAAAAA4AAwAAAAAAAQAAAA8A"
          "BQAAAAAABAAAAG1haW4AAAAAAwADAAIAAADCAQAABAAJAEdMX0FSQl9zZXBhcmF0ZV9zaGFkZXJfb2JqZWN0cwAA"
          "BAAKAEdMX0dPT0dMRV9jcHBfc3R5bGVfbGluZV9kaXJlY3RpdmUAAAQACABHTF9HT09HTEVfaW5jbHVkZV9kaXJl"
          "Y3RpdmUABQAEAAQAAABtYWluAAAAAAUABAAKAAAAdGV4MQAAAAAFAAQACwAAAHRleDIAAAAARwAEAAoAAAAiAAAA"
          "AwAAAEcABAAKAAAAIQAAAAMAAABHAAQACwAAACIAAAADAAAARwAEAAsAAAAhAAAAAwAAABMAAgACAAAAIQADAAMA"
          "AAACAAAAFgADAAYAAAAgAAAAGQAJAAcAAAAGAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAGwADAAgAAAAHAAAA"
          "IAAEAAkAAAAAAAAACAAAADsABAAJAAAACgAAAAAAAAA7AAQACQAAAAsAAAAAAAAANgAFAAIAAAAEAAAAAAAAAAMA"
          "AAD4AAIABQAAAP0AAQA4AAEA");
      writeFile(dir / "test.spv", rawContent);

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::ShaderSpvErr);
    });
  }

  SECTION("Loading malformed spir-v shader throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.spv", "Hello world");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.spv"), err::ShaderSpvErr);
    });
  }
}

} // namespace tria::asset::tests

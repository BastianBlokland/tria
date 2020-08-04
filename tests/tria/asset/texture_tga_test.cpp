#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/texture.hpp"
#include "tria/math/base64.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Texture Truevision TGA", "[asset]") {

  /*
   * The test images are exported from gimp 2.10.20 and then base64 encoded.
   */

  SECTION("2x2 upper-left uncompressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode(
              "AAACAAAAAAAAAAIAAgACABggAAD/AP8A/wAA////AAAAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA="));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 255}});
    });
  }

  SECTION("2x2 bottom-left uncompressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode(
              "AAACAAAAAAAAAAAAAgACABgA/wAA////AAD/AP8AAAAAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA="));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 255}});
    });
  }

  SECTION("2x2 with alpha upper-left uncompressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode(
              "AAACAAAAAAAAAAIAAgACACAoAAD//wD/AJP/AACT/////wAAAAAAAAAAVFJVRVZJU0lPTi1YRklMRS4A"));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 147}, {0, 0, 255, 147}, {255, 255, 255, 255}});
    });
  }

  SECTION("2x2 with alpha bottom-left uncompressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode(
              "AAACAAAAAAAAAAAAAgACACAI/wAAk/////8AAP//AP8AkwAAAAAAAAAAVFJVRVZJU0lPTi1YRklMRS4A"));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 147}, {0, 0, 255, 147}, {255, 255, 255, 255}});
    });
  }

  SECTION("4x4 upper-left rle-compressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode("AAAKAAAAAAAAAAQABAAEABggggAA/wAA/wCDAP8AAwD/AP8AAAAA/wD/AIH///"
                             "+BAAAAAAAAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA="));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{4, 4});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255},
              {255, 0, 0, 255},
              {255, 0, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 0, 255, 255},
              {255, 0, 0, 255},
              {0, 255, 0, 255},
              {255, 255, 255, 255},
              {255, 255, 255, 255},
              {0, 0, 0, 255},
              {0, 0, 0, 255},
          });
    });
  }

  SECTION("4x4 bottom-left rle-compressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode("AAAKAAAAAAAAAAAABAAEABgAgf///4EAAAADAP8A/wAAAAD/AP8AgwD/"
                             "AIIAAP8AAP8AAAAAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA="));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{4, 4});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255},
              {255, 0, 0, 255},
              {255, 0, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 0, 255, 255},
              {255, 0, 0, 255},
              {0, 255, 0, 255},
              {255, 255, 255, 255},
              {255, 255, 255, 255},
              {0, 0, 0, 255},
              {0, 0, 0, 255},
          });
    });
  }

  SECTION("4x4 with alpha upper-left rle-compressed") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga",
          math::base64Decode(
              "AAAKAAAAAAAAAAAABAAEACAIA/////////+oAAAA/wAAAJMDAP8Ak/8AAP8AAP+TAP8A/wMA/wD/AP8AkwD/"
              "AP8A/wCTAwAA/5MAAP//AAD/kwD/AP8AAAAAAAAAAFRSVUVWSVNJT04tWEZJTEUuAA=="));

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.tga")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{4, 4});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 147},
              {255, 0, 0, 255},
              {255, 0, 0, 147},
              {0, 255, 0, 255},
              {0, 255, 0, 255},
              {0, 255, 0, 147},
              {0, 255, 0, 255},
              {0, 255, 0, 147},
              {0, 255, 0, 147},
              {0, 0, 255, 255},
              {255, 0, 0, 147},
              {0, 255, 0, 255},
              {255, 255, 255, 255},
              {255, 255, 255, 168},
              {0, 0, 0, 255},
              {0, 0, 0, 147},
          });
    });
  }

  SECTION("Loading truncated uncompressed image throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.tga", math::base64Decode("AAACAAAAAAAAAAAAAgACACAI/wAAk/////8AAP//"));

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.tga"), err::AssetLoadErr);
    });
  }

  SECTION("Loading truncated rle-compressed image throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.tga", math::base64Decode("AAAKAAAAAAAAAAAABAAEACAIA/////////+oAAAATEUuAA=="));

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.tga"), err::AssetLoadErr);
    });
  }
}

} // namespace tria::asset::tests

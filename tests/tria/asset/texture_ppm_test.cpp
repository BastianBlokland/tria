#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/texture_ppm_err.hpp"
#include "tria/asset/texture.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - Texture Portable Pixmap", "[asset]") {

  SECTION("Formatted P3 ascii can be loaded") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P3\n"
          "2 2 255\n"
          "255 0 0\n"
          "0 255 0\n"
          "0 0 255\n"
          "128 128 128\n");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("Color per line P3 ascii can be loaded") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P3\n"
          "2\n2\n255\n"
          "255\n0\n0\n"
          "0\n255\n0\n"
          "0\n0\n255\n"
          "128\n128\n128\n");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("Single line P3 ascii can be loaded") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.ppm", "P3 2 2 255 255 0 0 0 255 0 0 0 255 128 128 128");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("Commented P3 ascii can be loaded") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "# Hello\nP3\n"
          "# Comments\n2# Are\n2#Supported\n255#Everywhere\n"
          "# In\n255# The\n0   # Format\n0 # Will\n"
          "# That\n0 # Parse\n255 # Correctly?\n0\n"
          "0 0 255\n"
          "128 128 128# End of file\n");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("Windows style line-endings are supported") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P3\r\n"
          "2 2 255\r\n"
          "# Comments with windows line-endings\r\n"
          "255 0 0\r\n"
          "0 255 0\r\n"
          "0 0 255\r\n"
          "128 128 128\r\n");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("P6 binary can be loaded") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P6 2 2 255\n"
          "\xFF\x1\x1"
          "\x1\xFF\x1"
          "\x1\x1\xFF"
          "\x80\x80\x80");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{
              {255, 1, 1, 255}, {1, 255, 1, 255}, {1, 1, 255, 255}, {128, 128, 128, 255}});
    });
  }

  SECTION("Loading with an invalid format type throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.ppm", "P9 1 1 255 255 255 255");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.ppm"), err::TexturePpmErr);
    });
  }

  SECTION("Loading with an invalid size throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.ppm", "P3 0 0 255 255 255 255");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.ppm"), err::TexturePpmErr);
    });
  }

  SECTION("Loading with an invalid bit-depth throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.ppm", "P3 0 0 128 128 128 128");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.ppm"), err::TexturePpmErr);
    });
  }

  SECTION("In P3 unspecified colors are treated as black") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P3 2 2 255\n"
          "255 0 0\n"
          "0 255 0");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}});
    });
  }

  SECTION("In P6 unspecified colors are treated as black") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P6 2 2 255\n"
          "\xFF\x1\x1"
          "\x1\xFF\x1"
          "\x1\x1\x1"
          "\x1\x1\x1");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{2, 2});
      CHECK(
          pixels ==
          std::vector<Pixel>{{255, 1, 1, 255}, {1, 255, 1, 255}, {1, 1, 1, 255}, {1, 1, 1, 255}});
    });
  }

  SECTION("In P3 extra colors are ignored") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P3 1 1 255\n"
          "255 0 0\n"
          "0 255 0\n"
          "0 0 255");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{1, 1});
      CHECK(pixels == std::vector<Pixel>{{255, 0, 0, 255}});
    });
  }

  SECTION("In P6 extra colors are ignored") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ppm",
          "P6 1 1 255\n"
          "\xFF\x1\x1"
          "\x1\xFF\x1"
          "\x1\x1\xFF");

      auto db     = Database{nullptr, dir};
      auto* tex   = db.get("test.ppm")->downcast<Texture>();
      auto pixels = std::vector<Pixel>(tex->getPixelBegin(), tex->getPixelEnd());
      CHECK(tex->getSize() == TextureSize{1, 1});
      CHECK(pixels == std::vector<Pixel>{{255, 1, 1, 255}});
    });
  }
}

} // namespace tria::asset::tests

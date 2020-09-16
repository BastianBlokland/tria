#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/font_ttf_err.hpp"
#include "tria/asset/font.hpp"
#include "tria/math/base64.hpp"
#include "utils.hpp"

namespace tria::asset::tests {

TEST_CASE("[asset] - True Type Font", "[asset]") {

  /*
   * The test fonts are exported from fontforge (git commit:
   * c3468cbd0320c152c0cbf762b9e2b63642d9c65f) and then base64 encoded.
   */

  SECTION("Simple test ttf") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.ttf",
          math::base64Decode(
              "AAEAAAAOAIAAAwBgRkZUTZKGfgsAAAXMAAAAHEdERUYAFQAUAAAFsAAAABxPUy8yYqNs7QAAAWgAAABgY21h"
              "cAAPA98AAAHYAAABQmN2dCAARAURAAADHAAAAARnYXNw//"
              "8AAwAABagAAAAIZ2x5Zo6zAJ8AAAMsAAAAdGhlYWQafppxAAAA7AAAADZoaGVhCiYIBQAAASQAAAAkaG10eB"
              "gABCwAAAHIAAAAEGxvY2EAZgBYAAADIAAAAAptYXhwAEgAOQAAAUgAAAAgbmFtZZKIeQUAAAOgAAAB0XBvc3"
              "TMWOidAAAFdAAAADQAAQAAAAEAAAxB/"
              "+9fDzz1AAsIAAAAAADbgiVLAAAAANuCKtQARAAABBgFVQAAAAgAAgAAAAAAAAABAAAFVQAAALgIAAAAAAAEG"
              "AABAAAAAAAAAAAAAAAAAAAABAABAAAABAAIAAIAAAAAAAIAAAABAAEAAABAAC4AAAAAAAQIAAGQAAUAAAUzB"
              "ZkAAAEeBTMFmQAAA9cAZgISAAACAAUJAAAAAAAAAAAAAQAAAAAAAAAAAAAAAFBmRWQAwAAxADEGZv5mALgFV"
              "QAAAAAAAQAAAAAAAAAAAAAAIAABCAAARAAAAAAIAAAACAAD6AAAAAMAAAADAAAAHAABAAAAAAA8AAMAAQAAA"
              "BwABAAgAAAABAAEAAEAAAAx//8AAAAx////"
              "0gABAAAAAAAAAQYAAAEAAAAAAAAAAQIAAAACAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAD"
              "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
              "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
              "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
              "AAAAAAAAAAAAAAAAAAAAAAAAAABEBREAAAAsACwALAA6AAAAAgBEAAACZAVVAAMABwAusQEALzyyBwQA7TKx"
              "BgXcPLIDAgDtMgCxAwAvPLIFBADtMrIHBgH8PLIBAgDtMjMRIRElIREhRAIg/"
              "iQBmP5oBVX6q0QEzQAAAAED6AAABBgEAAADAAAhETMRA+"
              "gwBAD8AAAAAAAAAA4ArgABAAAAAAAAABsAOAABAAAAAAABAAQAXgABAAAAAAACAAcAcwABAAAAAAADABwAtQ"
              "ABAAAAAAAEAAQA3AABAAAAAAAFABABAwABAAAAAAAGAAQBHgADAAEECQAAADYAAAADAAEECQABAAgAVAADAA"
              "EECQACAA4AYwADAAEECQADADgAewADAAEECQAEAAgA0gADAAEECQAFACAA4QADAAEECQAGAAgBFABDAG8AcA"
              "B5AHIAaQBnAGgAdAAgACgAYwApACAAMgAwADIAMAAsACAAYgBhAHMAdABpAGEAbgAAQ29weXJpZ2h0IChjKS"
              "AyMDIwLCBiYXN0aWFuAAB0AGUAcwB0AAB0ZXN0AABSAGUAZwB1AGwAYQByAABSZWd1bGFyAABGAG8AbgB0AE"
              "YAbwByAGcAZQAgADoAIAB0AGUAcwB0ACAAOgAgADEAMgAtADkALQAyADAAMgAwAABGb250Rm9yZ2UgOiB0ZX"
              "N0IDogMTItOS0yMDIwAAB0AGUAcwB0AAB0ZXN0AABWAGUAcgBzAGkAbwBuACAAMAAwADEALgAwADAAMAAgAA"
              "BWZXJzaW9uIDAwMS4wMDAgAAB0AGUAcwB0AAB0ZXN0AAAAAAACAAAAAAAA/"
              "2cAZgAAAAEAAAAAAAAAAAAAAAAAAAAAAAQAAAABAAIBAglnbHlwaF9vbmUAAAAB//"
              "8AAgABAAAAAAAAAAwAFAAEAAAAAgAAAAEAAAABAAAAAAABAAAAANuCLesAAAAA24IlSwAAAADbgirU"));

      auto db    = Database{nullptr, dir};
      auto* font = db.get("test.ttf")->downcast<Font>();

      const auto* glyph = font->getGlyph(0x31); // Get the 'digit one' glyph.
      REQUIRE(glyph);

      /* Glyph is a box consisting of 4 points and 4 lines connecting the edges of the box.
       */

      REQUIRE(glyph->getNumSegments() == 4U);
      CHECK(glyph->getSegmentsBegin()[0].type == GlyphSegmentType::Line);
      CHECK(glyph->getSegmentsBegin()[0].startPointIdx == 0U);

      CHECK(glyph->getSegmentsBegin()[1].type == GlyphSegmentType::Line);
      CHECK(glyph->getSegmentsBegin()[1].startPointIdx == 1U);

      CHECK(glyph->getSegmentsBegin()[2].type == GlyphSegmentType::Line);
      CHECK(glyph->getSegmentsBegin()[2].startPointIdx == 2U);

      CHECK(glyph->getSegmentsBegin()[3].type == GlyphSegmentType::Line);
      CHECK(glyph->getSegmentsBegin()[3].startPointIdx == 3U);

      CHECK(approx(glyph->getPoint(0U), {0.951020419f, 0.f}));
      CHECK(approx(glyph->getPoint(1U), {0.951020419f, 0.750183165f}));
      CHECK(approx(glyph->getPoint(2U), {1.f, 0.750183165f}));
      CHECK(approx(glyph->getPoint(3U), {1.f, 0.f}));
    });
  }
}

} // namespace tria::asset::tests

#include "tria/asset/database.hpp"
#include "tria/asset/font.hpp"
#include "tria/gfx/context.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/utils.hpp"
#include "tria/math/vec.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <cstring>
#include <thread>

using namespace std::literals;
using namespace tria;
using namespace tria::math;
using namespace tria::asset;
using namespace std::chrono;

auto xRoot(Vec2f p1, Vec2f p2) {
  // line equation: p = p1 + (p2 - p1) * t

  // 0      = p1.y + (p2.y - p1.y) * t
  // -p1.y  = (p2.y - p1.y) * t
  // -p1.y / (p2.y - p1.y) = t

  // t = p1.y / (p2.y - p1.y)

  const auto to2 = p2 - p1;
  if (to2.y() == 0.f) {
    // parallel line, no root.
    return -1.f;
  }
  const auto t = -p1.y() / to2.y();
  if (t < 0.f || t > 1.f) {
    return -1.f;
  }
  return p1.x() + to2.x() * t;
}

auto contains(Vec2f point, const Glyph* glyph) {
  auto num = 0U;
  for (auto itr = glyph->getSegmentsBegin(); itr != glyph->getSegmentsEnd(); ++itr) {
    switch (itr->type) {
    case GlyphSegmentType::Line: {
      const auto p1   = glyph->getPoint(itr->startPointIdx);
      const auto p2   = glyph->getPoint(itr->startPointIdx + 1U);
      const auto root = xRoot(p1 - point, p2 - point);
      num += root >= 0.f;
    } break;
    case GlyphSegmentType::QuadraticBezier: {
      const auto p1 = glyph->getPoint(itr->startPointIdx);
      // const auto c = glyph->getPoint(itr->startPointIdx + 1U);
      const auto p2   = glyph->getPoint(itr->startPointIdx + 2U);
      const auto root = xRoot(p1 - point, p2 - point);
      num += root >= 0.f;
    } break;
    }
  }
  return (num % 2U) == 1U;
}

auto runApp(pal::Platform& /*unused*/, asset::Database& db, gfx::Context& /*unused*/) {
  const auto* font  = db.get("fonts/hack_regular.ttf")->downcast<asset::Font>();
  const auto* glyph = font->getGlyph(0x42);
  const auto size   = 128U;

  using Rgb = math::Vec<uint8_t, 3>;

  auto pixels = PodVector<Rgb>{size * size};
  for (auto y = 0U; y != size; ++y) {
    for (auto x = 0U; x != size; ++x) {
      const auto point     = Vec2f{(x + .5f) / size, (y + .5f) / size};
      pixels[y * size + x] = contains(point, glyph) ? Rgb{255U, 255U, 255U} : Rgb{0U, 0U, 0U};
    }
  }

  const auto destPath = pal::getCurExecutablePath().replace_extension("ppm");
  auto* fileHandle    = std::fopen(destPath.u8string().c_str(), "wb");
  const auto* header  = "P6\n128 128\n255\n";
  std::fwrite(header, std::strlen(header), 1U, fileHandle);
  std::fwrite(pixels.data(), pixels.size() * sizeof(Rgb), 1U, fileHandle);
  std::fclose(fileHandle);

  return 0;
}

auto main(int /*unused*/, char* * /*unused*/) -> int {

  pal::setThreadName("tria_main_thread");

  auto logger = log::Logger{
      log::makeConsolePrettySink(),
      log::makeFileJsonSink(pal::getCurExecutablePath().replace_extension("log"))};

  int ret;
  try {
    auto platform = pal::Platform{&logger};
    auto db = asset::Database{&logger, pal::getCurExecutablePath().parent_path() / "fonttest_data"};
    auto gfx = gfx::Context{&logger};

    LOG_I(&logger, "FontTest startup");

    ret = runApp(platform, db, gfx);
  } catch (const std::exception& e) {
    LOG_E(&logger, "Uncaught exception", {"what", e.what()});
    ret = 1;
  } catch (...) {
    LOG_E(&logger, "Uncaught exception");
    ret = 1;
  }

  LOG_I(&logger, "FontTest shutdown");
  return ret;
}

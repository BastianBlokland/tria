#include "tria/asset/database.hpp"
#include "tria/asset/font.hpp"
#include "tria/gfx/context.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/utils.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <thread>

using namespace std::literals;
using namespace tria;
using namespace tria::math;
using namespace std::chrono;

auto drawGlyph(asset::Database& db, gfx::Canvas& canvas, const asset::Glyph* glyph, Box2f bounds) {
  static auto points = math::PodVector<math::Vec4f>{};

  for (auto c = 0U; c != glyph->getNumContours(); ++c) {
    points.clear();
    for (const auto* p = glyph->getContourBegin(c); p != glyph->getContourEnd(c); ++p) {
      if (p->type == asset::GlyphPointType::Normal) {
        points.emplace_back(
            lerp(bounds.min().x(), bounds.max().x(), p->position.x()),
            lerp(bounds.min().y(), bounds.max().y(), p->position.y()),
            0.f,
            0.f);
      }
    }
    // Add the first point again to 'close' the contour.
    points.emplace_back(
        lerp(bounds.min().x(), bounds.max().x(), glyph->getContourBegin(c)->position.x()),
        lerp(bounds.min().y(), bounds.max().y(), glyph->getContourBegin(c)->position.y()),
        0.f,
        0.f);

    canvas.draw(
        db.get("graphics/lines.gfx")->downcast<asset::Graphic>(),
        static_cast<uint32_t>(points.size()),
        points.data(),
        points.size() * sizeof(math::Vec4f),
        1U);
  }
}

auto runApp(pal::Platform& platform, asset::Database& db, gfx::Context& gfx) {

  auto win    = platform.createWindow({512, 512});
  auto canvas = gfx.createCanvas(
      &win,
      gfx::VSyncMode::Enable,
      gfx::SampleCount::X1,
      gfx::DepthMode::Disable,
      gfx::clearMask(gfx::Clear::Color));

  while (!win.getIsCloseRequested()) {
    platform.handleEvents();

    if (canvas.drawBegin()) {
      const auto* font    = db.get("fonts/hack_regular.ttf")->downcast<asset::Font>();
      const auto gridSize = 5U;

      for (auto y = 0U; y != gridSize; ++y) {
        for (auto x = 0U; x != gridSize; ++x) {
          auto* glyph = font->getGlyphBegin() + y * gridSize + x;
          if (glyph < font->getGlyphEnd()) {
            auto xNrm = x / static_cast<float>(gridSize);
            auto yNrm = y / static_cast<float>(gridSize);
            drawGlyph(
                db, canvas, glyph, {{xNrm, yNrm}, {xNrm + 1.f / gridSize, yNrm + 1.f / gridSize}});
          }
        }
      }
      canvas.drawEnd();

    } else {
      // Unable to draw, possibly due to a minimized window.
      std::this_thread::sleep_for(100ms);
    }
  }
  return 0;
}

auto main(int /*unused*/, char* * /*unused*/) -> int {

  pal::setThreadName("tria_main_thread");

  auto logger =
      log::Logger{log::makeConsolePrettySink(),
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

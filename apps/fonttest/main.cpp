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

auto quadBezier(math::Vec2f p0, math::Vec2f c, math::Vec2f p1, float t) {
  const auto invT = 1.f - t;
  return c + (p0 - c) * invT * invT + (p1 - c) * t * t;
}

[[nodiscard]] auto quadBezierRoots(Vec2f p0, Vec2f c, Vec2f p1) -> std::vector<float> {
  /* (1 - t)² * p0.y + 2 * t * (1 - t) * c.y + t² * p1.y = 0
   * t1 = -sqrt(c² - p0.y * p1.y + c.y - p0.y) / (2 * c.y - p0.y - p1.y)
   * t2 = sqrt(c² - p0.y * p1.y + c.y - p0.y) / (2 * c.y - p0.y - p1.y)
   */
  const auto d  = c.y() * c.y() - p0.y() * p1.y() + c.y() - p0.y();
  const auto v  = std::sqrt(d);
  const auto v2 = 2 * c.y() - p0.y() - p1.y();
  // if (d <= .0f) {
  //   return {};
  // }
  const auto ta = -v / v2;
  const auto tb = v / v2;
  auto result   = std::vector<float>{};
  if (ta >= 0.f && ta <= 1.f) {
    // const auto xa = (1 - ta) * (1 - ta) * p0.x() + 2 * ta * (1 - ta) * c.x() + ta * ta * p1.x();
    result.push_back(quadBezier(p0, c, p1, ta).x());
  }
  if (tb >= 0.f && tb <= 1.f) {
    // const auto xb = (1 - tb) * (1 - tb) * p0.x() + 2 * tb * (1 - tb) * c.x() + tb * tb * p1.x();
    result.push_back(quadBezier(p0, c, p1, tb).x());
  }
  return result;
}

auto plot(asset::Database& db, gfx::Canvas& canvas, math::Vec2f p0, math::Vec2f c, math::Vec2f p1) {
  static auto points = math::PodVector<math::Vec2f>{};
  points.clear();

  const auto numSegs = 100U;
  for (auto i = 0U; i != numSegs; ++i) {
    auto t = i / static_cast<float>(numSegs - 1U);
    auto p = quadBezier(p0, c, p1, t);
    if (i > 1U) {
      points.push_back(points.back());
    }
    points.emplace_back(p);
  }

  canvas.draw(
      db.get("graphics/lines.gfx")->downcast<asset::Graphic>(),
      static_cast<uint32_t>(points.size()),
      points.data(),
      points.size() * sizeof(math::Vec2f),
      1U);
}

auto drawCircle(asset::Database& db, gfx::Canvas& canvas, math::Vec2f p) {
  canvas.draw(db.get("graphics/circle.gfx")->downcast<asset::Graphic>(), p);
}

auto drawCircleRed(asset::Database& db, gfx::Canvas& canvas, math::Vec2f p) {
  canvas.draw(db.get("graphics/circle_red.gfx")->downcast<asset::Graphic>(), p);
}

auto drawLine(asset::Database& db, gfx::Canvas& canvas, math::Vec2f p0, math::Vec2f p1) {
  static auto points = math::PodVector<math::Vec2f>{};
  points.clear();

  points.push_back(p0);
  points.push_back(p1);

  canvas.draw(
      db.get("graphics/lines_gray.gfx")->downcast<asset::Graphic>(),
      static_cast<uint32_t>(points.size()),
      points.data(),
      points.size() * sizeof(math::Vec2f),
      1U);
}

auto runApp(pal::Platform& platform, asset::Database& db, gfx::Context& gfx) {
  auto win    = platform.createWindow({512, 512});
  auto canvas = gfx.createCanvas(
      &win,
      gfx::VSyncMode::Enable,
      gfx::SampleCount::X1,
      gfx::DepthMode::Disable,
      gfx::clearMask(gfx::Clear::Color));

  auto p0 = Vec2f{-.5f, 0.f};
  auto c  = Vec2f{+.0f, +.5f};
  auto p1 = Vec2f{+.5f, 0.f};

  while (!win.getIsCloseRequested()) {
    platform.handleEvents();
    if (canvas.drawBegin()) {

      if (win.isKeyDown(pal::Key::Alpha1)) {
        p0 = (win.getMousePosNrm() * 2.f - Vec2f{1.f, 1.f}) * Vec2f{1.f, -1.f};
      }
      if (win.isKeyDown(pal::Key::Alpha2)) {
        c = (win.getMousePosNrm() * 2.f - Vec2f{1.f, 1.f}) * Vec2f{1.f, -1.f};
      }
      if (win.isKeyDown(pal::Key::Alpha3)) {
        p1 = (win.getMousePosNrm() * 2.f - Vec2f{1.f, 1.f}) * Vec2f{1.f, -1.f};
      }

      drawLine(db, canvas, {-1, 0}, {1, 0});
      drawLine(db, canvas, {0, -1}, {0, 1});

      plot(db, canvas, p0, c, p1);
      drawCircle(db, canvas, p0);
      drawCircle(db, canvas, c);
      drawCircle(db, canvas, p1);

      for (const auto x : quadBezierRoots(p0, c, p1)) {
        drawCircleRed(db, canvas, {x, 0});
      }

      canvas.drawEnd();
    } else {
      // Unable to draw, possibly due to a minimized window.
      std::this_thread::sleep_for(100ms);
    }

    char titleBuffer[256];
    std::snprintf(
        titleBuffer,
        sizeof(titleBuffer),
        "p0: (%.2f, %.2f), c: (%.2f, %.2f), p1: (%.2f, %.2f)",
        p0.x(),
        p0.y(),
        c.x(),
        c.y(),
        p1.x(),
        p1.y());
    win.setTitle(titleBuffer);
  }
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

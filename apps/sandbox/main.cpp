#include "tria/asset/database.hpp"
#include "tria/gfx/context.hpp"
#include "tria/math/vec.hpp"
#include "tria/pal/interrupt.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <cmath>
#include <thread>

using namespace std::literals;
using namespace tria;
using Clock = std::chrono::high_resolution_clock;

auto runApp(pal::Platform& platform, asset::Database& db, gfx::Context& gfx) {

  auto win    = platform.createWindow({512, 512});
  auto canvas = gfx.createCanvas(&win, gfx::VSyncMode::Disable);

  const auto* triangle = db.get("triangle.gfx")->downcast<asset::Graphic>();
  const auto* quad     = db.get("quad.gfx")->downcast<asset::Graphic>();

  auto pos = math::Vec2f{0, 0};
  while (!win.getIsCloseRequested() && !pal::isInterruptRequested() &&
         !win.isKeyPressed(pal::Key::Escape)) {

    platform.handleEvents();

    pos += math::Vec2f(win.getScrollDelta()) * 0.01f;

    if (canvas.drawBegin(math::color::gray())) {

      canvas.draw(triangle, pos);

      const auto isMouseDown = win.isKeyDown(pal::Key::MouseLeft);
      const auto ndcMousePos = win.getMousePosNrm() * 2 - math::Vec2f{1, 1};
      canvas.draw(isMouseDown ? quad : triangle, math::Vec3f{ndcMousePos.x(), ndcMousePos.y(), 0});

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
  pal::setupInterruptHandler();

  auto logger = log::Logger{log::makeConsolePrettySink(), log::makeFileJsonSink("sandbox.log")};

  int ret;
  try {
    auto platform = pal::Platform{&logger};
    auto db       = asset::Database{&logger, pal::getCurExecutablePath().parent_path() / "data"};
    auto gfx      = gfx::Context{&logger};

    LOG_I(&logger, "Sandbox startup");

    ret = runApp(platform, db, gfx);
  } catch (const std::exception& e) {
    LOG_E(&logger, "Uncaught exception", {"what", e.what()});
    ret = 1;
  } catch (...) {
    LOG_E(&logger, "Uncaught exception");
    ret = 1;
  }

  LOG_I(&logger, "Sandbox shutdown");
  return ret;
}

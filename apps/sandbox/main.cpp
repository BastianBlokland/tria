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

  const auto* circle = db.get("circle.gfx")->downcast<asset::Graphic>();

  while (!win.getIsCloseRequested() && !pal::isInterruptRequested() &&
         !win.isKeyPressed(pal::Key::Escape)) {

    platform.handleEvents();

    // Toggle fullscreen when pressing '1'.
    if (win.isKeyPressed(pal::Key::Alpha1)) {
      if (win.getFullscreenMode() == pal::FullscreenMode::Disable) {
        win.setSize({0, 0}, pal::FullscreenMode::Enable);
      } else {
        win.setSize({512, 512}, pal::FullscreenMode::Disable);
      }
    }

    if (canvas.drawBegin(math::color::gray())) {

      canvas.draw(circle, math::Vec3f{0, 0, 0});

      const auto ndcMousePos = win.getMousePosNrm() * 2 - math::Vec2f{1, 1};
      canvas.draw(circle, math::Vec3f{ndcMousePos.x(), ndcMousePos.y(), 0});

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

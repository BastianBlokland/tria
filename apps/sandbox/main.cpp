#include "tria/gfx/context.hpp"
#include "tria/log/api.hpp"
#include "tria/pal/interrupt.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>

using namespace std::literals;
using namespace tria;

using Clock    = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double>;

auto operator<<(std::ostream& out, const Duration& rhs) -> std::ostream& {
  auto s = rhs.count();
  if (s < .001) {
    out << s * 1000000 << " us";
  } else {
    out << s * 1000 << " ms";
  }
  return out;
}

auto runApp(log::Logger& logger, pal::Platform& platform) -> int {

  auto gfxContext = gfx::Context{&logger};
  auto mainWin    = platform.createWindow(512, 512);
  auto mainCanvas = gfxContext.createCanvas(&mainWin, false);

  auto frameBegin = Clock::now();
  while (!mainWin.getIsCloseRequested() && !pal::isInterruptRequested()) {

    // Process platform events.
    platform.handleEvents();

    if (mainCanvas.drawBegin()) {
      // Do some actual drawing here :)
      mainCanvas.drawEnd();
    } else {
      // Unable to draw, possibly due to a minimized window.
      std::this_thread::sleep_for(100ms);
    }

    auto frameEnd = Clock::now();
    auto dur      = frameEnd - frameBegin;
    frameBegin    = frameEnd;

    // Update window title.
    std::stringstream titleStream;
    titleStream << "Sandbox - " << mainWin.getWidth() << "x" << mainWin.getHeight() << " " << dur;
    mainWin.setTitle(titleStream.str());
  }

  return 0;
}

auto main(int /*unused*/, char* * /*unused*/) -> int {

  pal::setThreadName("tria_main_thread");
  pal::setupInterruptHandler();

  auto logger = log::Logger{log::makeConsolePrettySink(), log::makeFileJsonSink("sandbox.log")};
  LOG_I(&logger, "Sandbox startup");

  auto platform = pal::Platform{&logger};
  try {
    runApp(logger, platform);
  } catch (const std::exception& e) {
    LOG_E(&logger, "Uncaught exception", {"what", e.what()});
  } catch (...) {
    LOG_E(&logger, "Uncaught exception");
  }

  LOG_I(&logger, "Sandbox shutdown");
  return 0;
}

#include "tria/log/api.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>

using namespace std::literals;
using namespace tria;

auto runApp(log::Logger& /*unused*/, pal::Platform& platform) -> int {

  auto mainWin = platform.createWindow(512, 512);
  while (!mainWin.getIsCloseRequested()) {

    // Process platform events.
    platform.handleEvents();

    // Update window title.
    std::stringstream titleStream;
    titleStream << "Tria sandbox - " << mainWin.getWidth() << "x" << mainWin.getHeight();
    mainWin.setTitle(titleStream.str());

    // Sleep until next 'frame'.
    std::this_thread::sleep_for(100ms);
  }

  return 0;
}

auto main(int /*unused*/, char* * /*unused*/) -> int {

  pal::setThreadName("main-thread");

  auto logger = log::Logger{log::makeConsolePrettySink(), log::makeFileJsonSink("sandbox.log")};
  LOG_I(&logger, "Sandbox startup");

  auto platform = pal::Platform{&logger, "Tria sandbox"};
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

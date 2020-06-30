#include "log/api.hpp"
#include "pal/platform.hpp"
#include "pal/utils.hpp"
#include <chrono>
#include <sstream>
#include <thread>

using namespace std::literals;

auto main(int /*unused*/, char* * /*unused*/) -> int {

  auto logger = log::Logger{log::makeConsolePrettySink(), log::makeFileJsonSink("sandbox.log")};
  LOG_I(&logger, "Sandbox init");

  pal::setThreadName("main-thread");
  auto platform = pal::Platform{&logger, "Tria sandbox"};
  auto& win     = platform.createWindow(512, 512);

  while (!win.getIsCloseRequested()) {

    // Process platform events.
    platform.handleEvents();

    // Update window title.
    std::stringstream titleStream;
    titleStream << "Tria test - " << win.getWidth() << "x" << win.getHeight();
    win.setTitle(titleStream.str());

    // Sleep until next 'frame'.
    std::this_thread::sleep_for(100ms);
  }

  LOG_I(&logger, "Sandbox teardown");
  return 0;
}

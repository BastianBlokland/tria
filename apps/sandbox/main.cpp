#include "pal/platform.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std::literals;

auto main(int /*unused*/, char* * /*unused*/) -> int {
  std::cout << "Sandbox init\n";

  auto platform = pal::Platform{"Tria sandbox"};
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

  std::cout << "Sandbox teardown\n";
  return 0;
}

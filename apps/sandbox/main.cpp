#include "pal/platform.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

auto main(int /*unused*/, char* * /*unused*/) -> int {
  std::cout << "Sandbox init\n";

  auto platform = pal::Platform{};
  auto& win     = platform.createWindow(256, 256);

  while (!win.getIsCloseRequested()) {

    // Process platform events.
    platform.handleEvents();

    // Update window title.
    std::stringstream titleStream;
    titleStream << "Tria test - " << win.getWidth() << "x" << win.getHeight();
    win.setTitle(titleStream.str());

    // Sleep until next 'frame'.
    usleep(100'000);
  }

  std::cout << "Sandbox teardown\n";
  return 0;
}

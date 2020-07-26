#pragma once
#include "tria/pal/key.hpp"
#include "tria/pal/window.hpp"

namespace tria::pal::internal {

/* Structure of per-window input data.
 */
struct WinInput final {
  WindowPos mousePos;
  KeyMask downKeys;
  bool isCloseRequested;
};

} // namespace tria::pal::internal

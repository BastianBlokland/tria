#pragma once
#include "../defines.hpp"

#if PLATFORM_LINUX
#include "window.linux.xcb.hpp"
#elif PLATFORM_WINDOWS
#include "window.win32.hpp"
#else
static_assert(false, "Unsupported platform");
#endif

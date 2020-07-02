#pragma once
#include "tria/defines.hpp"

#if PLATFORM_LINUX
#include "platform.linux.xcb.hpp"
#elif PLATFORM_WINDOWS
#include "platform.win32.hpp"
#else
static_assert(false, "Unsupported platform");
#endif

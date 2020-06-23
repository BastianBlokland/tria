#pragma once
#include "../defines.hpp"

#if PLATFORM_LINUX
#include "platform.linux.xcb.hpp"
#else
static_assert(false, "Unsupported platform");
#endif

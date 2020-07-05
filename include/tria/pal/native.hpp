#include "tria/pal/window.hpp"

/*
 * This header can be used to retreive the low-level platform information about windows.
 * Note: Only include this if it is absolutely required as it will drag in big platform headers.
 */

#if defined(TRIA_LINUX_XCB)
#include <xcb/xcb.h>

[[nodiscard]] auto getLinuxXcbConnection(const Window& window) noexcept -> xcb_connection_t*;
[[nodiscard]] auto getLinuxXcbWindow(const Window& window) noexcept -> xcb_window_t;

#endif

#if defined(TRIA_WIN32)
#include <windows.h>

[[nodiscard]] auto getWin32HInstance(const Window& window) noexcept -> HINSTANCE;
[[nodiscard]] auto getWin32HWnd(const Window& window) noexcept -> HWND;

#endif

#include "pal/platform.win32.hpp"
#include "pal/err/display_protocol_err.hpp"
#include "pal/err/window_err.hpp"
#include <algorithm>
#include <iostream>

namespace pal {

constexpr auto windowMinWidth  = 128;
constexpr auto windowMinHeight = 128;

auto WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT {

  /* When creating a window we pass a pointer to the platform that created it, then when we receive
   * 'WM_NCCREATE' event we store that pointer to the window's 'GWLP_USERDATA' storage. This way for
   * every event we can lookup what platform should handle it. */

  if (msg == WM_NCCREATE) {
    auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
    auto* platform     = static_cast<Platform*>(createStruct->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(platform));
  } else {
    // Retreive the platform pointer.
    auto* platform = reinterpret_cast<Platform*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // Let the platform handle the event, if the platform returns 'false' we execute the default
    // window behaviour for that event.
    if (platform->handleEvent(hWnd, msg, wParam, lParam)) {
      return 0;
    }
  }

  // Default window behaviour if platform didn't consume the message.
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

static auto getWin32ErrorMsg(unsigned long errCode) noexcept -> std::string {
  // Translate the errCode into a string message.
  LPSTR msgBuffer = nullptr;
  const auto size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errCode,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&msgBuffer),
      0,
      nullptr);

  // Copy the buffer to a string object.
  auto result = std::string{msgBuffer, size};

  // Free the win32 buffer.
  LocalFree(msgBuffer);

  return result;
}

static auto throwDisplayProtocolError() {
  const auto errCode = GetLastError();
  throw err::DisplayProtocolErr{errCode, getWin32ErrorMsg(errCode)};
}

Platform::~Platform() {
  while (!m_windows.empty()) {
    destroyWindow(m_windows.back());
  }
}

auto Platform::handleEvents() -> void {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

auto Platform::createWindow(uint16_t width, uint16_t height) -> Window& {
  if (!m_hInstance) {
    win32Setup();
  }

  // Increment a counter to create a unique id for this window.
  ++m_winCreateCounter;

  // Create a unique class-name for this window class.
  auto className = m_appName;
  className += std::to_string(m_winCreateCounter);

  // Create a window-class structure.
  WNDCLASSEX winClass    = {};
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW;
  winClass.lpfnWndProc   = WindowProc;
  winClass.hInstance     = m_hInstance;
  winClass.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
  winClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  winClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
  winClass.lpszClassName = className.c_str();
  winClass.hIconSm       = LoadIcon(nullptr, IDI_WINLOGO);

  // Register the window-class to win32.
  if (!RegisterClassEx(&winClass)) {
    throwDisplayProtocolError();
  }

  const DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  // Calculate the size of the window (because we give width and height of the content area).
  RECT winRect   = {};
  winRect.right  = static_cast<long>(width);
  winRect.bottom = static_cast<long>(height);
  AdjustWindowRect(&winRect, dwStyle, false);

  // Create a new window.
  auto winHandle = CreateWindow(
      className.c_str(),
      nullptr,
      dwStyle,
      0,
      0,
      winRect.right - winRect.left,
      winRect.bottom - winRect.top,
      nullptr,
      nullptr,
      m_hInstance,
      static_cast<LPVOID>(this));
  if (!winHandle) {
    throwDisplayProtocolError();
  }

  // Center on screen.
  const auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
  const auto x            = (screenWidth - winRect.right) / 2;
  const auto y            = (screenHeight - winRect.bottom) / 2;
  SetWindowPos(winHandle, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  // Show and focus window.
  ShowWindow(winHandle, SW_SHOW);
  SetForegroundWindow(winHandle);
  SetFocus(winHandle);

  // Keep track of the window.
  m_windows.push_back(Window{winHandle, std::move(className), dwStyle, width, height});
  return m_windows.back();
}

auto Platform::destroyWindow(const Window& win) -> void {
  // Find the window in our windows list.
  const auto winItr = std::find_if(m_windows.begin(), m_windows.end(), [&win](const Window& elem) {
    return win.m_winHandle == elem.m_winHandle;
  });
  if (winItr == m_windows.end()) {
    throw err::WindowErr{};
  }

  // Destroy the win32 window and remove the window class.
  DestroyWindow(win.m_winHandle);
  UnregisterClass(win.m_winClassName.c_str(), m_hInstance);

  // Remove the window from our windows list.
  m_windows.erase(winItr);
}

auto Platform::win32Setup() -> void {
  m_hInstance = GetModuleHandle(nullptr);
  if (!m_hInstance) {
    throwDisplayProtocolError();
  }
}

auto Platform::getWindow(HWND winHandle) noexcept -> Window* {
  const auto winItr =
      std::find_if(m_windows.begin(), m_windows.end(), [winHandle](const Window& elem) {
        return elem.m_winHandle == winHandle;
      });
  return winItr == m_windows.end() ? nullptr : &(*winItr);
}

auto Platform::handleEvent(HWND hWnd, UINT msg, WPARAM /*unused*/, LPARAM lParam) noexcept -> bool {
  switch (msg) {
  case WM_CLOSE: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->m_isCloseRequested = true;
      return true;
    }
    return false;
  }
  case WM_SIZE: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->m_width  = LOWORD(lParam);
      window->m_height = HIWORD(lParam);
      return true;
    }
    return false;
  }
  case WM_GETMINMAXINFO: {
    auto minMaxInfo              = reinterpret_cast<LPMINMAXINFO>(lParam);
    minMaxInfo->ptMinTrackSize.x = windowMinWidth;
    minMaxInfo->ptMinTrackSize.y = windowMinHeight;
    return true;
  }
  case WM_PAINT: {
    ValidateRect(hWnd, nullptr);
    return true;
  }
  default:
    return false;
  }
}

} // namespace pal

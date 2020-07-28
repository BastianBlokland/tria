#include "native_platform.win32.hpp"
#include "internal/win32_utils.hpp"
#include "tria/pal/utils.hpp"
#include "tria/pal/window.hpp"
#include <array>

namespace tria::pal {

auto WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT {

  /* When creating a window we pass a pointer to the NativePlatform that created it, then when we
   * receive 'WM_NCCREATE' event we store that pointer to the window's 'GWLP_USERDATA' storage. This
   * way for every event we can lookup what platform should handle it. */

  if (msg == WM_NCCREATE) {
    auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
    auto* platform     = static_cast<NativePlatform*>(createStruct->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(platform));
  } else {
    // Retreive the platform pointer.
    auto* platform = reinterpret_cast<NativePlatform*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // Let the platform handle the event, if the platform returns 'false' we execute the default
    // window behaviour for that event.
    if (platform->handleEvent(hWnd, msg, wParam, lParam)) {
      return 0;
    }
  }
  // Default window behaviour if platform didn't consume the message.
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

NativePlatform::NativePlatform(log::Logger* logger) :
    m_logger{logger}, m_appName{getCurExecutableName()}, m_nextWinId{1} {

  LOG_I(logger, "Platform init", {"executable", m_appName}, {"pid", getCurProcessId()});

  win32Setup();
}

NativePlatform::~NativePlatform() {
  while (!m_windows.empty()) {
    destroyWindow(m_windows.begin()->first);
  }
}

auto NativePlatform::handleEvents() -> void {

  // Reset any events (like pressed keys) from the previous 'handleEvents' call.
  resetEvents();

  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

auto NativePlatform::createWindow(WindowSize desiredSize) -> Window {
  auto winId = m_nextWinId++;

  // Create a unique class-name for this window class.
  auto className = m_appName;
  className += std::to_string(winId);

  const auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

  if (desiredSize.x() == 0) {
    desiredSize.x() = screenWidth;
  }
  if (desiredSize.y() == 0) {
    desiredSize.y() = screenHeight;
  }

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
    internal::throwPlatformError();
  }

  const DWORD dwStyle           = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
  const DWORD dwFullscreenStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  // Calculate the size of the window (because we give width and height of the content area).
  RECT desiredWinRect   = {};
  desiredWinRect.right  = static_cast<long>(desiredSize.x());
  desiredWinRect.bottom = static_cast<long>(desiredSize.y());
  AdjustWindowRect(&desiredWinRect, dwStyle, false);

  // Create a new window.
  auto winHandle = CreateWindow(
      className.c_str(),
      nullptr,
      dwStyle,
      0,
      0,
      desiredWinRect.right - desiredWinRect.left,
      desiredWinRect.bottom - desiredWinRect.top,
      nullptr,
      nullptr,
      m_hInstance,
      static_cast<LPVOID>(this));
  if (!winHandle) {
    internal::throwPlatformError();
  }

  // Get the content size that the windowmanager gave the window (might not match the desired size).
  RECT realClientRect;
  if (!GetClientRect(winHandle, &realClientRect)) {
    internal::throwPlatformError();
  }
  WindowSize realSize = {
      realClientRect.right - realClientRect.left, realClientRect.bottom - realClientRect.top};

  // Get window size of the window (including borders).
  RECT realWinRect;
  if (!GetWindowRect(winHandle, &realWinRect)) {
    internal::throwPlatformError();
  }

  // Center on screen.
  const auto x = (screenWidth - realWinRect.right) / 2;
  const auto y = (screenHeight - realWinRect.bottom) / 2;
  SetWindowPos(winHandle, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  // Show and focus window.
  ShowWindow(winHandle, SW_SHOW);
  SetForegroundWindow(winHandle);
  SetFocus(winHandle);

  LOG_I(
      m_logger,
      "Window created",
      {"id", winId},
      {"desiredSize", desiredSize},
      {"realSize", realSize});

  // Keep track of the window data.
  m_windows.insert(
      {winId,
       WindowData{winId, winHandle, std::move(className), dwStyle, dwFullscreenStyle, realSize}});

  // Return a handle to the window.
  return Window{this, winId};
}

auto NativePlatform::destroyWindow(WindowId id) noexcept -> void {
  auto* winData = getWindow(id);
  assert(winData);

  // Destroy the win32 window and remove the window class.
  DestroyWindow(winData->handle);
  UnregisterClass(winData->className.c_str(), m_hInstance);

// Remove the window data.
#if defined(NDEBUG)
  m_windows.erase(id);
#else
  auto erased = m_windows.erase(id);
  assert(erased);
#endif

  LOG_I(m_logger, "Window destroyed", {"id", id});
}

auto NativePlatform::setWinTitle(WindowId id, std::string_view title) noexcept -> void {
  auto* winData = getWindow(id);
  assert(winData);
  SetWindowText(winData->handle, title.data());
}

auto NativePlatform::setWinSize(WindowId id, WindowSize desiredSize, FullscreenMode fullscreen)
    -> bool {

  auto* winData = getWindow(id);
  assert(winData);

  if (desiredSize.x() == 0) {
    desiredSize.x() = GetSystemMetrics(SM_CXSCREEN);
  }
  if (desiredSize.y() == 0) {
    desiredSize.y() = GetSystemMetrics(SM_CYSCREEN);
  }

  LOG_D(
      m_logger,
      "Updating window size",
      {"id", id},
      {"desiredSize", desiredSize},
      {"fullscreen", getName(fullscreen)});

  switch (fullscreen) {
  case FullscreenMode::Enable:
    // TODO(bastian): Investigate supporting different sizes in fullscreen, requires actually
    // changing the system display settings.

    // Update the window style.
    SetWindowLongPtr(winData->handle, GWL_STYLE, winData->dwFullscreenStyle);

    // Present the updated window.
    ShowWindow(winData->handle, SW_MAXIMIZE);
    break;

  case FullscreenMode::Disable:
  default:

    // Update the window style.
    SetWindowLongPtr(winData->handle, GWL_STYLE, winData->dwStyle);

    // Calculate the size of the window (because we give width and height of the content area).
    RECT winRect   = {};
    winRect.right  = static_cast<long>(desiredSize.x());
    winRect.bottom = static_cast<long>(desiredSize.y());
    AdjustWindowRect(&winRect, winData->dwStyle, false);

    // Set the window size.
    SetWindowPos(
        winData->handle,
        nullptr,
        0,
        0,
        winRect.right - winRect.left,
        winRect.bottom - winRect.top,
        SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);

    // Present the updated window.
    ShowWindow(winData->handle, SW_RESTORE);
    break;
  }

  // Update the local size with the real content size of the window (in case the window-manager gave
  // us a different size then was desired).
  RECT realClientRect;
  if (GetClientRect(winData->handle, &realClientRect)) {
    winData->size = {
        realClientRect.right - realClientRect.left, realClientRect.bottom - realClientRect.top};
  }

  return true;
}

auto NativePlatform::win32Setup() -> void {
  m_hInstance = GetModuleHandle(nullptr);
  if (!m_hInstance) {
    internal::throwPlatformError();
  }

  LOG_I(
      m_logger,
      "Win32 init",
      {"screenSize", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)});
}

auto NativePlatform::handleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    -> bool {
  switch (msg) {
  case WM_CLOSE: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.requestClose();
      return true;
    }
    return false;
  }
  case WM_SIZE: {
    auto* window = getWindow(hWnd);
    if (window) {
      const auto newWidth  = LOWORD(lParam);
      const auto newHeight = HIWORD(lParam);
      const auto newSize   = WindowSize{newWidth, newHeight};
      if (newSize != window->size) {
        LOG_D(m_logger, "Window resized", {"id", window->id}, {"size", newSize});
      }
      window->size = newSize;
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
  case WM_MOUSEMOVE: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.setMousePos({LOWORD(lParam), HIWORD(lParam)});
    }
    return window;
  }
  case WM_LBUTTONDOWN: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markPressed(Key::MouseLeft);
    }
    return window;
  }
  case WM_RBUTTONDOWN: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markPressed(Key::MouseRight);
    }
    return window;
  }
  case WM_MBUTTONDOWN: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markPressed(Key::MouseMiddle);
    }
    return window;
  }
  case WM_LBUTTONUP: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markReleased(Key::MouseLeft);
    }
    return window;
  }
  case WM_RBUTTONUP: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markReleased(Key::MouseRight);
    }
    return window;
  }
  case WM_MBUTTONUP: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->input.markReleased(Key::MouseMiddle);
    }
    return window;
  }
  case WM_KEYDOWN: {
    auto* window   = getWindow(hWnd);
    const auto key = internal::winVkToKey(wParam);
    if (window && key) {
      window->input.markPressed(*key);
    }
    return window;
  }
  case WM_KEYUP: {
    auto* window   = getWindow(hWnd);
    const auto key = internal::winVkToKey(wParam);
    if (window && key) {
      window->input.markReleased(*key);
    }
    return window;
  }
  case WM_MOUSEWHEEL: {
    auto* window = getWindow(hWnd);
    if (window) {
      const auto delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
      window->input.updateScroll({0, delta});
    }
    return window;
  }
  case WM_MOUSEHWHEEL: {
    auto* window = getWindow(hWnd);
    if (window) {
      const auto delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
      window->input.updateScroll({-delta, 0});
    }
    return window;
  }
  default:
    return false;
  }
}

auto NativePlatform::resetEvents() noexcept -> void {
  for (auto& win : m_windows) {
    win.second.input.reset();
  }
}

auto NativePlatform::getWindow(HWND handle) noexcept -> WindowData* {
  // TODO(bastian): If we ever have a significant amount of windows we should add a faster lookup
  // then this linear scan.
  for (auto& kvp : m_windows) {
    if (kvp.second.handle == handle) {
      return &kvp.second;
    }
  }
  return nullptr;
}

auto NativePlatform::getWindow(WindowId id) noexcept -> WindowData* {
  auto itr = m_windows.find(id);
  if (itr == m_windows.end()) {
    return nullptr;
  }
  return &itr->second;
}

auto NativePlatform::getWindow(WindowId id) const noexcept -> const WindowData* {
  auto itr = m_windows.find(id);
  if (itr == m_windows.end()) {
    return nullptr;
  }
  return &itr->second;
}

[[nodiscard]] auto getWin32HInstance(const Window& window) noexcept -> HINSTANCE {
  return window.getNativePlatformPtr()->getHInstance();
}

[[nodiscard]] auto getWin32HWnd(const Window& window) noexcept -> HWND {
  return window.getNativePlatformPtr()->getHWnd(window.getWindowId());
}

} // namespace tria::pal

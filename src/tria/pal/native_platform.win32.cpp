#include "native_platform.win32.hpp"
#include "tria/pal/err/platform_err.hpp"
#include "tria/pal/utils.hpp"
#include <array>

namespace tria::pal {

namespace {

auto getWin32ErrorMsg(unsigned long errCode) noexcept -> std::string {
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

auto throwPlatformError() {
  const auto errCode = GetLastError();
  throw err::PlatformErr{errCode, getWin32ErrorMsg(errCode)};
}

} // namespace

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
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

auto NativePlatform::createWindow(uint16_t width, uint16_t height) -> Window {
  auto winId = m_nextWinId++;

  // Create a unique class-name for this window class.
  auto className = m_appName;
  className += std::to_string(winId);

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
    throwPlatformError();
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
    throwPlatformError();
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

  LOG_I(m_logger, "Window created", {"id", winId}, {"width", width}, {"height", height});

  // Keep track of the window data.
  m_windows.insert(
      {winId, WindowData{winId, winHandle, std::move(className), dwStyle, width, height}});

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

auto NativePlatform::setWinSize(WindowId id, uint16_t width, uint16_t height) noexcept -> void {
  auto* winData = getWindow(id);
  assert(winData);

  // Calculate the size of the window (because we give width and height of the content area).
  RECT winRect   = {};
  winRect.right  = static_cast<long>(width);
  winRect.bottom = static_cast<long>(height);
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
}

auto NativePlatform::win32Setup() -> void {
  m_hInstance = GetModuleHandle(nullptr);
  if (!m_hInstance) {
    throwPlatformError();
  }

  LOG_I(
      m_logger,
      "Win32 init",
      {"screenWidth", GetSystemMetrics(SM_CXSCREEN)},
      {"screenHeight", GetSystemMetrics(SM_CYSCREEN)});
}

auto NativePlatform::handleEvent(HWND hWnd, UINT msg, WPARAM /*unused*/, LPARAM lParam) noexcept
    -> bool {
  switch (msg) {
  case WM_CLOSE: {
    auto* window = getWindow(hWnd);
    if (window) {
      window->isCloseRequested = true;
      return true;
    }
    return false;
  }
  case WM_SIZE: {
    auto* window = getWindow(hWnd);
    if (window) {
      const auto newWidth  = LOWORD(lParam);
      const auto newHeight = HIWORD(lParam);
      if (newWidth != window->width || newHeight != window->height) {
        LOG_D(
            m_logger,
            "Window resized",
            {"id", window->id},
            {"width", newWidth},
            {"height", newHeight});
      }
      window->width  = newWidth;
      window->height = newHeight;
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

auto NativePlatform::getWindow(HWND handle) noexcept -> WindowData* {
  // TODO: If we ever have a significant amount of windows we should add a faster lookup then this
  // linear scan.
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

#include "native_platform.xcb.hpp"
#include "tria/pal/err/platform_err.hpp"
#include "tria/pal/utils.hpp"
#include <array>

namespace tria::pal {

NativePlatform::NativePlatform(log::Logger* logger) : m_logger{logger}, m_xcbScreen{nullptr} {

  LOG_I(
      logger, "Platform init", {"executable", getCurExecutableName()}, {"pid", getCurProcessId()});

  xcbSetup();
}

NativePlatform::~NativePlatform() {
  while (!m_windows.empty()) {
    destroyWindow(m_windows.begin()->first);
  }
  xcbTeardown();
}

auto NativePlatform::handleEvents() -> void {

  xcb_generic_event_t* evt;
  while ((evt = xcb_poll_for_event(m_xcbCon))) {
    switch (evt->response_type & ~0x80) {
    case XCB_CLIENT_MESSAGE: {
      const auto* clientMsg = static_cast<xcb_client_message_event_t*>(static_cast<void*>(evt));
      auto* winData         = getWindow(clientMsg->window);
      if (!winData) {
        // Unknown window.
        break;
      }
      // User wants to close the window.
      if (clientMsg->data.data32[0] == m_xcbDeleteMsgAtom) {
        winData->isCloseRequested = true;
      }
      break;
    }
    case XCB_CONFIGURE_NOTIFY: {
      const auto* configMsg = static_cast<xcb_configure_notify_event_t*>(static_cast<void*>(evt));
      auto* winData         = getWindow(configMsg->window);
      if (!winData) {
        // Unknown window.
        break;
      }

      const auto newSize = WindowSize{configMsg->width, configMsg->height};
      if (newSize != winData->size) {
        LOG_D(m_logger, "Window resized", {"id", configMsg->window}, {"size", newSize});
      }
      winData->size = newSize;
    } break;
    default:
      // Unknown event.
      break;
    }
    std::free(evt);
  }
}

auto NativePlatform::createWindow(WindowSize size) -> Window {

  const auto winId    = xcb_generate_id(m_xcbCon);
  const auto eventMsk = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  const auto valList =
      std::array<uint32_t, 2>{m_xcbScreen->black_pixel, XCB_EVENT_MASK_STRUCTURE_NOTIFY};

  if (size.x() == 0) {
    size.x() = m_xcbScreen->width_in_pixels;
  }
  if (size.y() == 0) {
    size.y() = m_xcbScreen->height_in_pixels;
  }

  // Create a window on the xcb side.
  xcb_create_window(
      m_xcbCon,
      XCB_COPY_FROM_PARENT,
      winId,
      m_xcbScreen->root,
      0,
      0,
      size.x(),
      size.y(),
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      m_xcbScreen->root_visual,
      eventMsk,
      valList.data());

  // Register a custom delete message atom.
  xcb_change_property(
      m_xcbCon, XCB_PROP_MODE_REPLACE, winId, m_xcbProtoMsgAtom, 4, 32, 1, &m_xcbDeleteMsgAtom);

  // Show the window.
  xcb_map_window(m_xcbCon, winId);
  xcb_flush(m_xcbCon);

  LOG_I(m_logger, "Window created", {"id", winId}, {"size", size});

  // Keep track of the window data.
  m_windows.insert({winId, WindowData{winId, size}});

  // Return a handle to the window.
  return Window{this, winId};
}

auto NativePlatform::destroyWindow(WindowId id) noexcept -> void {
  // Remove the window data.
#if defined(NDEBUG)
  m_windows.erase(id);
#else
  auto erased = m_windows.erase(id);
  assert(erased);
#endif

  // Destroy the xcb window it belongs to.
  xcb_destroy_window(m_xcbCon, id);
  xcb_flush(m_xcbCon);

  LOG_I(m_logger, "Window destroyed", {"id", id});
}

auto NativePlatform::setWinTitle(WindowId id, std::string_view title) noexcept -> void {
  xcb_change_property(
      m_xcbCon,
      XCB_PROP_MODE_REPLACE,
      id,
      XCB_ATOM_WM_NAME,
      XCB_ATOM_STRING,
      8,
      title.size(),
      title.data());
  xcb_flush(m_xcbCon);
}

auto NativePlatform::setWinSize(WindowId id, const WindowSize size) noexcept -> void {
  const auto valList = std::array<uint32_t, 2>{size.x(), size.y()};
  xcb_configure_window(
      m_xcbCon, id, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, valList.data());

  xcb_flush(m_xcbCon);

  // Update local information immediately.
  auto* win = getWindow(id);
  assert(win);
  win->size = size;
}

auto NativePlatform::xcbSetup() -> void {
  // Connect to the xcb server.
  auto screenNum = 0;
  m_xcbCon       = xcb_connect(nullptr, &screenNum);
  xcbCheckErr();

  // Find the screen for our connection.
  const auto* setup = xcb_get_setup(m_xcbCon);
  auto rootItr      = xcb_setup_roots_iterator(setup);
  for (auto s = screenNum; s > 0; --s) {
    xcb_screen_next(&rootItr);
  }
  m_xcbScreen = rootItr.data;

  // Retrieve message atoms to listen for.
  m_xcbProtoMsgAtom  = xcbGetAtom("WM_PROTOCOLS");
  m_xcbDeleteMsgAtom = xcbGetAtom("WM_DELETE_WINDOW");

  LOG_I(
      m_logger,
      "Xcb connected",
      {"screenNum", screenNum},
      {"screenSize", m_xcbScreen->width_in_pixels, m_xcbScreen->height_in_pixels});
}

auto NativePlatform::xcbTeardown() noexcept -> void {
  xcb_disconnect(m_xcbCon);
  LOG_I(m_logger, "Xcb disconnected");
}

auto NativePlatform::xcbCheckErr() -> void {
  assert(m_xcbCon);
  const auto err = xcb_connection_has_error(m_xcbCon);
  if (err != 0) {
    std::string msg;
    switch (err) {
    case XCB_CONN_ERROR:
      msg = "x11: Connection error";
      break;
    case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
      msg = "x11: Extension not supported";
      break;
    case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
      msg = "x11: Insufficient memory available";
      break;
    case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
      msg = "x11: Request length exceeded";
      break;
    case XCB_CONN_CLOSED_PARSE_ERR:
      msg = "x11: Failed to parse display string";
      break;
    case XCB_CONN_CLOSED_INVALID_SCREEN:
      msg = "x11: No valid screen available";
      break;
    default:
      msg = "x11: Unknown error";
      break;
    }
    throw err::PlatformErr{static_cast<unsigned long>(err), msg};
  }
}

auto NativePlatform::xcbGetAtom(const std::string& name) noexcept -> xcb_atom_t {
  const auto req  = xcb_intern_atom(m_xcbCon, 0, name.length(), name.data());
  auto* reply     = xcb_intern_atom_reply(m_xcbCon, req, nullptr);
  const auto atom = reply->atom;
  std::free(reply);
  return atom;
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

[[nodiscard]] auto getLinuxXcbConnection(const Window& window) noexcept -> xcb_connection_t* {
  return window.getNativePlatformPtr()->getConnection();
}

auto getLinuxXcbWindow(const Window& window) noexcept -> xcb_window_t {
  return window.getWindowId();
}

} // namespace tria::pal

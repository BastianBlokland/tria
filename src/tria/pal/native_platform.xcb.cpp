#include "native_platform.xcb.hpp"
#include "internal/xcb_utils.hpp"
#include "tria/pal/err/platform_err.hpp"
#include "tria/pal/utils.hpp"
#include <array>

namespace tria::pal {

NativePlatform::NativePlatform(log::Logger* logger) : m_logger{logger}, m_xcbScreen{nullptr} {
  LOG_I(
      logger, "Platform init", {"executable", getCurExecutableName()}, {"pid", getCurProcessId()});

  xcbSetup();
  xkbSetup();
}

NativePlatform::~NativePlatform() {
  while (!m_windows.empty()) {
    destroyWindow(m_windows.begin()->first);
  }
  xcbTeardown();
}

auto NativePlatform::handleEvents() -> void {

  // Reset any events (like pressed keys) from the previous 'handleEvents' call.
  resetEvents();

  xcb_generic_event_t* evt;
  while ((evt = xcb_poll_for_event(m_xcbCon))) {
    switch (evt->response_type & ~0x80) {

    case XCB_CLIENT_MESSAGE: {
      const auto* clientMsg = static_cast<xcb_client_message_event_t*>(static_cast<void*>(evt));
      auto* winData         = getWindow(clientMsg->window);
      if (winData && clientMsg->data.data32[0] == m_xcbDeleteMsgAtom) {
        winData->input.requestClose();
      }
    } break;

    case XCB_CONFIGURE_NOTIFY: {
      const auto* configMsg = static_cast<xcb_configure_notify_event_t*>(static_cast<void*>(evt));
      auto* winData         = getWindow(configMsg->window);
      if (winData) {
        const auto newSize = WindowSize{configMsg->width, configMsg->height};
        if (newSize != winData->size) {
          LOG_D(m_logger, "Window resized", {"id", configMsg->window}, {"size", newSize});
        }
        winData->size = newSize;
      }
    } break;

    case XCB_MOTION_NOTIFY: {
      const auto* motionMsg = static_cast<xcb_motion_notify_event_t*>(static_cast<void*>(evt));
      auto* winData         = getWindow(motionMsg->event);
      if (winData) {
        winData->input.setMousePos({motionMsg->event_x, motionMsg->event_y});
      }
    } break;

    case XCB_BUTTON_PRESS: {
      const auto* pressMsg = static_cast<xcb_button_press_event_t*>(static_cast<void*>(evt));
      auto* winData        = getWindow(pressMsg->event);
      if (winData) {
        switch (pressMsg->detail) {
        case XCB_BUTTON_INDEX_1:
          winData->input.markPressed(Key::MouseLeft);
          break;
        case XCB_BUTTON_INDEX_2:
          winData->input.markPressed(Key::MouseMiddle);
          break;
        case XCB_BUTTON_INDEX_3:
          winData->input.markPressed(Key::MouseRight);
          break;
        }
      }
    } break;

    case XCB_BUTTON_RELEASE: {
      const auto* releaseMsg = static_cast<xcb_button_release_event_t*>(static_cast<void*>(evt));
      auto* winData          = getWindow(releaseMsg->event);
      if (winData) {
        switch (releaseMsg->detail) {
        case XCB_BUTTON_INDEX_1:
          winData->input.markReleased(Key::MouseLeft);
          break;
        case XCB_BUTTON_INDEX_2:
          winData->input.markReleased(Key::MouseMiddle);
          break;
        case XCB_BUTTON_INDEX_3:
          winData->input.markReleased(Key::MouseRight);
          break;
        }
      }
    } break;

    case XCB_KEY_PRESS: {
      const auto* pressMsg = static_cast<xcb_key_press_event_t*>(static_cast<void*>(evt));
      auto* winData        = getWindow(pressMsg->event);
      const auto key       = internal::xcbKeyCodeToKey(pressMsg->detail);
      if (winData && key) {
        winData->input.markPressed(*key);
      }
    } break;

    case XCB_KEY_RELEASE: {
      const auto* releaseMsg = static_cast<xcb_key_release_event_t*>(static_cast<void*>(evt));
      auto* winData          = getWindow(releaseMsg->event);
      const auto key         = internal::xcbKeyCodeToKey(releaseMsg->detail);
      if (winData && key) {
        winData->input.markReleased(*key);
      }
    } break;

    default:
      // Unknown event.
      break;
    }
    std::free(evt);
  }
}

auto NativePlatform::createWindow(WindowSize size) -> Window {

  const auto winId          = xcb_generate_id(m_xcbCon);
  const auto valMask        = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  const auto backPixelColor = m_xcbScreen->black_pixel;
  const auto evtMask        = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_BUTTON_PRESS |
      XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_KEY_PRESS |
      XCB_EVENT_MASK_KEY_RELEASE;
  const auto valList = std::array<uint32_t, 2>{
      backPixelColor,
      evtMask,
  };

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
      valMask,
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
  const auto valList = std::array<uint32_t, 2>{
      size.x(),
      size.y(),
  };
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

auto NativePlatform::xkbSetup() const noexcept -> bool {

  auto useExt = XCB_CALL_WITH_REPLY(
      m_xcbCon, xcb_xkb_use_extension, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  if (useExt.hasError()) {
    // TODO(bastian): Decide if we should throw here, the program still runs however the keyboard
    // input wont work correctly.
    LOG_E(m_logger, "Failed to initialize xkb", {"errorCode", useExt.getErrCode()});
    return false;
  }
  auto serverVerMajor = useExt.getValue()->serverMajor;
  auto serverVerMinor = useExt.getValue()->serverMinor;

  // Enable 'detectableAutoRepeat': by default x-server will send repeated press and release events
  // when holding down a key, we don't want this.
  auto setFlags = XCB_CALL_WITH_REPLY(
      m_xcbCon,
      xcb_xkb_per_client_flags,
      XCB_XKB_ID_USE_CORE_KBD,
      XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
      XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
      0,
      0,
      0);
  if (setFlags.hasError()) {
    // TODO(bastian): Decide if we should throw here, the program still runs however the keyboard
    // input wont work correctly.
    LOG_E(
        m_logger, "Failed to enable 'detectableAutoRepeat'", {"errorCode", setFlags.getErrCode()});
    return false;
  }

  LOG_I(m_logger, "Initialized xkb", {"version", serverVerMajor, serverVerMinor});
  return true;
}

auto NativePlatform::xcbTeardown() noexcept -> void {
  xcb_disconnect(m_xcbCon);
  LOG_I(m_logger, "Xcb disconnected");
}

auto NativePlatform::xcbCheckErr() -> void {
  assert(m_xcbCon);
  const auto err = xcb_connection_has_error(m_xcbCon);
  if (err != 0) {
    throw err::PlatformErr{static_cast<unsigned long>(err), internal::xcbErrToStr(err)};
  }
}

auto NativePlatform::xcbGetAtom(const std::string& name) noexcept -> xcb_atom_t {
  auto atomReply = XCB_CALL_WITH_REPLY(m_xcbCon, xcb_intern_atom, 0, name.length(), name.data());
  return atomReply.getValue()->atom;
}

auto NativePlatform::resetEvents() noexcept -> void {
  for (auto& [id, data] : m_windows) {
    data.input.reset();
  }
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

#include "pal/platform.linux.xcb.hpp"
#include "pal/err/display_protocol_err.hpp"
#include "pal/err/window_err.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace pal {

Platform::~Platform() {
  if (m_xcbCon) {
    while (!m_windows.empty()) {
      destroyWindow(m_windows.back());
    }
    xcbTeardown();
  }
}

auto Platform::handleEvents() -> void {
  xcb_generic_event_t* evt;
  while ((evt = xcb_poll_for_event(m_xcbCon))) {
    switch (evt->response_type & ~0x80) {
    case XCB_CLIENT_MESSAGE: {
      const auto* clientMsg = static_cast<xcb_client_message_event_t*>(static_cast<void*>(evt));
      auto* window          = getWindow(clientMsg->window);
      if (!window) {
        // Unknown window.
        break;
      }
      // User wants to close the window.
      if (clientMsg->data.data32[0] == m_xcbDeleteMsgAtom) {
        window->m_isCloseRequested = true;
      }
      break;
    }
    case XCB_CONFIGURE_NOTIFY: {
      const auto* configMsg = static_cast<xcb_configure_notify_event_t*>(static_cast<void*>(evt));
      auto* window          = getWindow(configMsg->window);
      if (!window) {
        // Unknown window.
        break;
      }
      window->m_x      = configMsg->x;
      window->m_y      = configMsg->y;
      window->m_width  = configMsg->width;
      window->m_height = configMsg->height;
    } break;
    default:
      // Unknown event.
      break;
    }
    std::free(evt);
  }
}

auto Platform::createWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height) -> Window& {
  if (!m_xcbCon) {
    xcbSetup();
  }

  const auto winXId   = xcb_generate_id(m_xcbCon);
  const auto eventMsk = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  const auto valList =
      std::array<uint32_t, 2>{m_xcbScreen->black_pixel, XCB_EVENT_MASK_STRUCTURE_NOTIFY};

  if (width == 0) {
    width = m_xcbScreen->width_in_pixels;
  }
  if (height == 0) {
    height = m_xcbScreen->height_in_pixels;
  }

  // Create a window on the xcb side.
  xcb_create_window(
      m_xcbCon,
      XCB_COPY_FROM_PARENT,
      winXId,
      m_xcbScreen->root,
      x,
      y,
      width,
      height,
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      m_xcbScreen->root_visual,
      eventMsk,
      valList.data());

  // Register a custom delete message atom.
  xcb_change_property(
      m_xcbCon, XCB_PROP_MODE_REPLACE, winXId, m_xcbProtoMsgAtom, 4, 32, 1, &m_xcbDeleteMsgAtom);

  // Show the window.
  xcb_map_window(m_xcbCon, winXId);

  xcb_flush(m_xcbCon);

  // Keep track of the window.
  m_windows.push_back(Window{m_xcbCon, winXId, x, y, width, height});
  return m_windows.back();
}

auto Platform::destroyWindow(const Window& win) -> void {

  // Find the window in our windows list.
  const auto winItr = std::find_if(m_windows.begin(), m_windows.end(), [&win](const Window& elem) {
    return win.m_xcbWin == elem.m_xcbWin;
  });
  if (winItr == m_windows.end()) {
    throw err::WindowErr{};
  }

  // Destroy the xcb window it belongs to.
  xcb_destroy_window(m_xcbCon, win.m_xcbWin);

  xcb_flush(m_xcbCon);

  // Remove the window from our windows list.
  m_windows.erase(winItr);
}

auto Platform::xcbSetup() -> void {

  // Connect to the xcb server.
  auto screenNum = 0;
  m_xcbCon       = xcb_connect(nullptr, &screenNum);
  xcbCheckErr();

  // Find the screen for our connection.
  const auto* setup = xcb_get_setup(m_xcbCon);
  auto rootItr      = xcb_setup_roots_iterator(setup);
  for (auto s = screenNum; s > 0; --s)
    xcb_screen_next(&rootItr);
  m_xcbScreen = rootItr.data;

  // Retrieve message atoms to listen for.
  m_xcbProtoMsgAtom  = xcbGetAtom("WM_PROTOCOLS");
  m_xcbDeleteMsgAtom = xcbGetAtom("WM_DELETE_WINDOW");
}

auto Platform::xcbTeardown() noexcept -> void { xcb_disconnect(m_xcbCon); }

auto Platform::xcbCheckErr() -> void {
  if (!m_xcbCon) {
    return;
  }
  const auto err = xcb_connection_has_error(m_xcbCon);
  if (err != 0) {
    throw err::DisplayProtocolErr{err};
  }
}

auto Platform::xcbGetAtom(const std::string& name) noexcept -> xcb_atom_t {
  const auto req  = xcb_intern_atom(m_xcbCon, 0, name.length(), name.data());
  auto* reply     = xcb_intern_atom_reply(m_xcbCon, req, nullptr);
  const auto atom = reply->atom;
  std::free(reply);
  return atom;
}

auto Platform::getWindow(xcb_window_t xcbWin) noexcept -> Window* {
  const auto winItr =
      std::find_if(m_windows.begin(), m_windows.end(), [xcbWin](const Window& elem) {
        return elem.m_xcbWin == xcbWin;
      });
  return winItr == m_windows.end() ? nullptr : &(*winItr);
}

} // namespace pal

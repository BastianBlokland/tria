#pragma once
#include "tria/pal/key.hpp"
#include <optional>
#include <string_view>
#include <xcb/xcb.h>

namespace tria::pal::internal {

[[nodiscard]] constexpr auto xcbErrToStr(int xcbErrCode) -> std::string_view {
  switch (xcbErrCode) {
  case XCB_CONN_ERROR:
    return "x11: Connection error";
    break;
  case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
    return "x11: Extension not supported";
    break;
  case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
    return "x11: Insufficient memory available";
    break;
  case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
    return "x11: Request length exceeded";
    break;
  case XCB_CONN_CLOSED_PARSE_ERR:
    return "x11: Failed to parse display string";
    break;
  case XCB_CONN_CLOSED_INVALID_SCREEN:
    return "x11: No valid screen available";
    break;
  default:
    return "x11: Unknown error";
    break;
  }
}

[[nodiscard]] constexpr auto xcbKeyCodeToKey(xcb_keycode_t xcbKeyCode) -> std::optional<Key> {
  switch (xcbKeyCode) {
  case 0x32:
  case 0x3E:
    return Key::Shift;
  case 0x25:
  case 0x69:
    return Key::Control;
  case 0x16:
    return Key::Backspace;
  case 0x77:
    return Key::Delete;
  case 0x17:
    return Key::Tab;
  case 0x31:
    return Key::Tilde;
  case 0x24:
    return Key::Return;
  case 0x9:
    return Key::Escape;
  case 0x41:
    return Key::Space;
  case 0x6F:
    return Key::ArrowUp;
  case 0x74:
    return Key::ArrowDown;
  case 0x72:
    return Key::ArrowRight;
  case 0x71:
    return Key::ArrowLeft;

  case 0x26:
    return Key::A;
  case 0x38:
    return Key::B;
  case 0x36:
    return Key::C;
  case 0x28:
    return Key::D;
  case 0x1A:
    return Key::E;
  case 0x29:
    return Key::F;
  case 0x2A:
    return Key::G;
  case 0x2B:
    return Key::H;
  case 0x1F:
    return Key::I;
  case 0x2C:
    return Key::J;
  case 0x2D:
    return Key::K;
  case 0x2E:
    return Key::L;
  case 0x3A:
    return Key::M;
  case 0x39:
    return Key::N;
  case 0x20:
    return Key::O;
  case 0x21:
    return Key::P;
  case 0x18:
    return Key::Q;
  case 0x1B:
    return Key::R;
  case 0x27:
    return Key::S;
  case 0x1C:
    return Key::T;
  case 0x1E:
    return Key::U;
  case 0x37:
    return Key::V;
  case 0x19:
    return Key::W;
  case 0x35:
    return Key::X;
  case 0x1D:
    return Key::Y;
  case 0x34:
    return Key::Z;

  case 0x13:
    return Key::Alpha0;
  case 0xA:
    return Key::Alpha1;
  case 0xB:
    return Key::Alpha2;
  case 0xC:
    return Key::Alpha3;
  case 0xD:
    return Key::Alpha4;
  case 0xE:
    return Key::Alpha5;
  case 0xF:
    return Key::Alpha6;
  case 0x10:
    return Key::Alpha7;
  case 0x11:
    return Key::Alpha8;
  case 0x12:
    return Key::Alpha9;
  }
  return std::nullopt;
}

} // namespace tria::pal::internal

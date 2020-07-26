#pragma once
#include "tria/pal/err/platform_err.hpp"
#include "tria/pal/key.hpp"
#include <optional>
#include <string_view>
#include <windows.h>

namespace tria::pal::internal {

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

[[nodiscard]] constexpr auto winVkToKey(WPARAM param) -> std::optional<Key> {
  switch (param) {
  case VK_SHIFT:
    return Key::Shift;
  case VK_CONTROL:
    return Key::Control;
  case VK_BACK:
    return Key::Backspace;
  case VK_DELETE:
    return Key::Delete;
  case VK_TAB:
    return Key::Tab;
  case 0xC0:
    return Key::Tilde;
  case VK_RETURN:
    return Key::Return;
  case VK_ESCAPE:
    return Key::Escape;
  case VK_SPACE:
    return Key::Space;
  case VK_UP:
    return Key::ArrowUp;
  case VK_DOWN:
    return Key::ArrowDown;
  case VK_RIGHT:
    return Key::ArrowRight;
  case VK_LEFT:
    return Key::ArrowLeft;

  case 0x41:
    return Key::A;
  case 0x42:
    return Key::B;
  case 0x43:
    return Key::C;
  case 0x44:
    return Key::D;
  case 0x45:
    return Key::E;
  case 0x46:
    return Key::F;
  case 0x47:
    return Key::G;
  case 0x48:
    return Key::H;
  case 0x49:
    return Key::I;
  case 0x4A:
    return Key::J;
  case 0x4B:
    return Key::K;
  case 0x4C:
    return Key::L;
  case 0x4D:
    return Key::M;
  case 0x4E:
    return Key::N;
  case 0x4F:
    return Key::O;
  case 0x50:
    return Key::P;
  case 0x51:
    return Key::Q;
  case 0x52:
    return Key::R;
  case 0x53:
    return Key::S;
  case 0x54:
    return Key::T;
  case 0x55:
    return Key::U;
  case 0x56:
    return Key::V;
  case 0x57:
    return Key::W;
  case 0x58:
    return Key::X;
  case 0x59:
    return Key::Y;
  case 0x5A:
    return Key::Z;

  case 0x30:
    return Key::Alpha0;
  case 0x31:
    return Key::Alpha1;
  case 0x32:
    return Key::Alpha2;
  case 0x33:
    return Key::Alpha3;
  case 0x34:
    return Key::Alpha4;
  case 0x35:
    return Key::Alpha5;
  case 0x36:
    return Key::Alpha6;
  case 0x37:
    return Key::Alpha7;
  case 0x38:
    return Key::Alpha8;
  case 0x39:
    return Key::Alpha9;
  }
  return std::nullopt;
}

} // namespace tria::pal::internal

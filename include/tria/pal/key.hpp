#pragma once
#include <cstdint>
#include <string_view>

namespace tria::pal {

using KeyMask = uint64_t;

/* Enumeration of keys.
 * Note: Physical location on keyboard might differ depending on user locale.
 */
enum class Key : uint64_t {
  MouseLeft   = 1ULL << 0U,
  MouseRight  = 1ULL << 1U,
  MouseMiddle = 1ULL << 2U,

  Shift      = 1ULL << 3U,
  Control    = 1ULL << 4U,
  Backspace  = 1ULL << 5U,
  Delete     = 1ULL << 6U,
  Tab        = 1ULL << 7U,
  Tilde      = 1ULL << 8U,
  Return     = 1ULL << 9U,
  Escape     = 1ULL << 10U,
  Space      = 1ULL << 11U,
  ArrowUp    = 1ULL << 12U,
  ArrowDown  = 1ULL << 13U,
  ArrowRight = 1ULL << 14U,
  ArrowLeft  = 1ULL << 15U,

  A = 1ULL << 16U,
  B = 1ULL << 17U,
  C = 1ULL << 18U,
  D = 1ULL << 19U,
  E = 1ULL << 20U,
  F = 1ULL << 21U,
  G = 1ULL << 22U,
  H = 1ULL << 23U,
  I = 1ULL << 24U,
  J = 1ULL << 25U,
  K = 1ULL << 26U,
  L = 1ULL << 27U,
  M = 1ULL << 28U,
  N = 1ULL << 29U,
  O = 1ULL << 30U,
  P = 1ULL << 31U,
  Q = 1ULL << 32U,
  R = 1ULL << 33U,
  S = 1ULL << 34U,
  T = 1ULL << 35U,
  U = 1ULL << 36U,
  V = 1ULL << 37U,
  W = 1ULL << 38U,
  X = 1ULL << 39U,
  Y = 1ULL << 40U,
  Z = 1ULL << 41U,

  Alpha0 = 1ULL << 42U,
  Alpha1 = 1ULL << 43U,
  Alpha2 = 1ULL << 44U,
  Alpha3 = 1ULL << 45U,
  Alpha4 = 1ULL << 46U,
  Alpha5 = 1ULL << 47U,
  Alpha6 = 1ULL << 48U,
  Alpha7 = 1ULL << 49U,
  Alpha8 = 1ULL << 50U,
  Alpha9 = 1ULL << 51U,
};

[[nodiscard]] constexpr auto operator|(Key lhs, Key rhs) noexcept -> KeyMask {
  return KeyMask(lhs) | KeyMask(rhs);
}

[[nodiscard]] constexpr auto operator|(KeyMask lhs, Key rhs) noexcept -> KeyMask {
  return lhs | KeyMask(rhs);
}

[[nodiscard]] constexpr auto isInMask(KeyMask mask, Key key) noexcept {
  return (mask & KeyMask(key)) != 0;
}

[[nodiscard]] constexpr auto getName(Key key) noexcept -> std::string_view {
  switch (key) {
  case Key::MouseLeft:
    return "mouse-left";
  case Key::MouseRight:
    return "mouse-right";
  case Key::MouseMiddle:
    return "mouse-middle";

  case Key::Shift:
    return "shift";
  case Key::Control:
    return "control";
  case Key::Backspace:
    return "backspace";
  case Key::Delete:
    return "delete";
  case Key::Tab:
    return "tab";
  case Key::Tilde:
    return "tilde";
  case Key::Return:
    return "return";
  case Key::Escape:
    return "escape";
  case Key::Space:
    return "space";
  case Key::ArrowUp:
    return "arrow-up";
  case Key::ArrowDown:
    return "arrow-down";
  case Key::ArrowRight:
    return "arrow-right";
  case Key::ArrowLeft:
    return "arrow-left";

  case Key::A:
    return "a";
  case Key::B:
    return "b";
  case Key::C:
    return "c";
  case Key::D:
    return "d";
  case Key::E:
    return "e";
  case Key::F:
    return "f";
  case Key::G:
    return "g";
  case Key::H:
    return "h";
  case Key::I:
    return "i";
  case Key::J:
    return "j";
  case Key::K:
    return "k";
  case Key::L:
    return "l";
  case Key::M:
    return "m";
  case Key::N:
    return "n";
  case Key::O:
    return "o";
  case Key::P:
    return "p";
  case Key::Q:
    return "q";
  case Key::R:
    return "r";
  case Key::S:
    return "s";
  case Key::T:
    return "t";
  case Key::U:
    return "u";
  case Key::V:
    return "v";
  case Key::W:
    return "w";
  case Key::X:
    return "x";
  case Key::Y:
    return "y";
  case Key::Z:
    return "z";

  case Key::Alpha0:
    return "alpha-0";
  case Key::Alpha1:
    return "alpha-1";
  case Key::Alpha2:
    return "alpha-2";
  case Key::Alpha3:
    return "alpha-3";
  case Key::Alpha4:
    return "alpha-4";
  case Key::Alpha5:
    return "alpha-5";
  case Key::Alpha6:
    return "alpha-6";
  case Key::Alpha7:
    return "alpha-7";
  case Key::Alpha8:
    return "alpha-8";
  case Key::Alpha9:
    return "alpha-9";
  }
  return "unknown";
}

} // namespace tria::pal

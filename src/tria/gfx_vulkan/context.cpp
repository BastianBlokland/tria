#include "native_canvas.hpp"
#include "native_context.hpp"
#include <stdexcept>

namespace tria::gfx {

Context::Context(log::Logger* logger) : m_native{std::make_unique<NativeContext>(logger)} {}

Context::~Context() = default;

auto Context::createCanvas(const pal::Window* window, VSyncMode vSync) -> Canvas {
  if (!window) {
    throw std::invalid_argument{"Window cannot be null"};
  }
  return Canvas{m_native->createCanvas(window, vSync)};
}

} // namespace tria::gfx

#include "native_context.hpp"
#include "native_surface.hpp"
#include <stdexcept>

namespace tria::gfx {

Context::Context(log::Logger* logger) : m_native{std::make_unique<NativeContext>(logger)} {}

Context::~Context() = default;

auto Context::createSurface(const pal::Window* window) -> Surface {
  if (!window) {
    throw std::invalid_argument{"Window cannot be null"};
  }
  return Surface{m_native->createSurface(window)};
}

} // namespace tria::gfx

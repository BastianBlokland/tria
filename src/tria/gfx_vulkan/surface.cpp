#include "native_surface.hpp"

namespace tria::gfx {

Surface::Surface(std::unique_ptr<NativeSurface> native) : m_native{std::move(native)} {}

Surface::~Surface() = default;

} // namespace tria::gfx

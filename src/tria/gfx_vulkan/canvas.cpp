#include "tria/gfx/canvas.hpp"
#include "native_canvas.hpp"

namespace tria::gfx {

Canvas::Canvas(std::unique_ptr<NativeCanvas> native) : m_native{std::move(native)} {}

Canvas::~Canvas() = default;

auto Canvas::drawBegin(math::Color clearCol) -> bool { return m_native->drawBegin(clearCol); }

auto Canvas::draw(const asset::Graphic* asset, const void* uniData, size_t uniSize) -> void {
  m_native->draw(asset, uniData, uniSize);
}

auto Canvas::drawEnd() -> void { m_native->drawEnd(); }

} // namespace tria::gfx

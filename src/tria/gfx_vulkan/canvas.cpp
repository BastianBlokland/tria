#include "tria/gfx/canvas.hpp"
#include "native_canvas.hpp"

namespace tria::gfx {

Canvas::Canvas(std::unique_ptr<NativeCanvas> native) : m_native{std::move(native)} {}

Canvas::~Canvas() = default;

auto Canvas::drawBegin() -> bool { return m_native->drawBegin(); }

auto Canvas::draw(const asset::Graphic* asset) -> void { m_native->draw(asset); }

auto Canvas::drawEnd() -> void { m_native->drawEnd(); }

} // namespace tria::gfx

#include "tria/gfx/canvas.hpp"
#include "native_canvas.hpp"

namespace tria::gfx {

Canvas::Canvas(std::unique_ptr<NativeCanvas> native) : m_native{std::move(native)} {}

Canvas::~Canvas() = default;

auto Canvas::getDrawStats() const noexcept -> DrawStats { return m_native->getDrawStats(); }

auto Canvas::drawBegin(math::Color clearCol) -> bool { return m_native->drawBegin(clearCol); }

auto Canvas::bindGlobalData(const void* data, size_t dataSize) -> void {
  return m_native->bindGlobalData(data, dataSize);
}

auto Canvas::draw(
    const asset::Graphic* asset,
    uint32_t indexCount,
    const void* instData,
    size_t instDatSize,
    uint32_t count) -> void {
  m_native->draw(asset, indexCount, instData, instDatSize, count);
}

auto Canvas::drawEnd() -> void { m_native->drawEnd(); }

} // namespace tria::gfx

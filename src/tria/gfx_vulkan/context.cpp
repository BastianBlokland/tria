#include "native_context.hpp"

namespace tria::gfx {

Context::Context(log::Logger* logger) : m_native{std::make_unique<NativeContext>(logger)} {}

Context::~Context() = default;

} // namespace tria::gfx

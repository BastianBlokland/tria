#pragma once
#include "graphic.hpp"
#include "tria/log/api.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>

namespace tria::gfx::internal {

class Device;
class ShaderManager;

class GraphicManager final {
public:
  GraphicManager(log::Logger* logger, const Device* device, ShaderManager* shaderManager) :
      m_logger{logger}, m_device{device}, m_shaderManager{shaderManager} {
    assert(m_device);
    assert(m_shaderManager);
  }
  ~GraphicManager() = default;

  [[nodiscard]] auto getGraphic(const asset::Graphic* asset, VkRenderPass vkRenderPass) noexcept
      -> const Graphic&;

private:
  log::Logger* m_logger;
  const Device* m_device;
  ShaderManager* m_shaderManager;
  std::unordered_map<const asset::Graphic*, Graphic> m_data;
};

using GraphicManagerUnique = std::unique_ptr<GraphicManager>;

} // namespace tria::gfx::internal

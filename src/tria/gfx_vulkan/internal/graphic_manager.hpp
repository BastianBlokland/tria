#pragma once
#include "device.hpp"
#include "graphic.hpp"
#include "tria/log/api.hpp"
#include <memory>
#include <unordered_map>

namespace tria::gfx::internal {

class GraphicManager final {
public:
  GraphicManager(log::Logger* logger, const Device* device) : m_logger{logger}, m_device{device} {}
  ~GraphicManager() = default;

  [[nodiscard]] auto getGraphic(const asset::Graphic* asset, VkRenderPass vkRenderPass) noexcept
      -> const Graphic&;

private:
  log::Logger* m_logger;
  const Device* m_device;
  std::unordered_map<const asset::Graphic*, GraphicUnique> m_data;
};

using GraphicManagerUnique = std::unique_ptr<GraphicManager>;

} // namespace tria::gfx::internal

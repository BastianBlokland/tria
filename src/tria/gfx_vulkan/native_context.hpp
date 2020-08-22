#pragma once
#include "internal/debug_messenger.hpp"
#include "tria/gfx/context.hpp"
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace tria::gfx {

class NativeCanvas;

/*
 * Implementation of a graphics 'Context' abstraction.
 */
class NativeContext final {
public:
  NativeContext(log::Logger* logger);
  NativeContext(const NativeContext& rhs)     = delete;
  NativeContext(NativeContext&& rhs) noexcept = delete;
  ~NativeContext();

  auto operator=(const NativeContext& rhs) -> NativeContext& = delete;
  auto operator=(NativeContext&& rhs) noexcept -> NativeContext& = delete;

  [[nodiscard]] auto getVkInstance() const noexcept { return m_vkInstance; }

  [[nodiscard]] auto createCanvas(
      const pal::Window* window,
      VSyncMode vSync,
      SampleCount samples,
      DepthMode depth,
      ClearMask clear) -> std::unique_ptr<NativeCanvas>;

  auto setDebugName(
      VkDevice vkDevice, VkObjectType vkType, uint64_t vkHandle, std::string_view name) const
      noexcept -> void;

  auto beginDebugLabel(VkCommandBuffer vkCmdBuffer, std::string_view msg, math::Color color) const
      noexcept -> void;
  auto endDebugLabel(VkCommandBuffer vkCmdBuffer) const noexcept -> void;

private:
  log::Logger* m_logger;
  std::string m_appName;
  VkInstance m_vkInstance;
  internal::DebugMessengerUnique m_dbgMessenger;
};

} // namespace tria::gfx

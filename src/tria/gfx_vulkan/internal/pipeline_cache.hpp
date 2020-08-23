#pragma once
#include "tria/log/api.hpp"
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

/* Load a pipeline cache, can be used to significantly speedup pipline creation.
 * Either loads an existing pipeline cache from disk or creates a new one.
 */
[[nodiscard]] auto
loadVkPipelineCache(log::Logger* logger, VkDevice vkDevice, const VkPhysicalDeviceProperties& props)
    -> VkPipelineCache;

/* Save a pipeline cache to disk to be able to speedup future startups.
 */
auto saveVkPipelineCache(log::Logger* logger, VkDevice vkDevice, VkPipelineCache vkPipelineCache)
    -> void;

} // namespace tria::gfx::internal

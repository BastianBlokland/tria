#pragma once
#include "device.hpp"
#include "raw_asset.hpp"
#include "tria/fs.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class ShaderAsset final {
public:
  ShaderAsset(const Device* device, const RawAsset& rawAsset);
  ~ShaderAsset();

  [[nodiscard]] auto getModule() const noexcept -> const VkShaderModule& { return m_module; }

private:
  const Device* m_device;
  VkShaderModule m_module;
};

using ShaderAssetPtr = std::unique_ptr<ShaderAsset>;

[[nodiscard]] auto loadShaderAsset(const Device* device, const fs::path& path) -> ShaderAssetPtr;

} // namespace tria::gfx::internal

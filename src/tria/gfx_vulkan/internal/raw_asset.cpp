#include "raw_asset.hpp"
#include "tria/gfx/err/asset_err.hpp"
#include <fstream>

namespace tria::gfx::internal {

[[nodiscard]] auto loadRawAsset(const fs::path& path) -> RawAssetPtr {
  auto file = std::ifstream{path.string(), std::ios::ate | std::ios::binary};
  if (!file.is_open()) {
    throw err::AssetErr{path, "Failed to load"};
  }
  const auto fileSize = static_cast<size_t>(file.tellg());
  auto buffer         = std::vector<char>(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return std::make_unique<RawAsset>(std::move(buffer));
}

} // namespace tria::gfx::internal

#include "loader.hpp"
#include "tria/asset/raw_asset.hpp"

namespace tria::asset::internal {

auto loadRawAsset(
    log::Logger* /*unused*/,
    DatabaseImpl* /*unused*/,
    AssetId id,
    const fs::path& /*unused*/,
    RawData raw) -> AssetUnique {

  return std::make_unique<RawAsset>(std::move(id), std::move(raw));
}

} // namespace tria::asset::internal

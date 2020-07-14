#include "loader.hpp"
#include "tria/asset/raw_asset.hpp"

namespace tria::asset::internal {

auto loadRawAsset(AssetId id, const fs::path& /*unused*/, RawData raw, DatabaseImpl * /*unused*/)
    -> AssetUnique {
  return std::make_unique<RawAsset>(std::move(id), std::move(raw));
}

} // namespace tria::asset::internal

#pragma once
#include "../database_impl.hpp"
#include "tria/fs.hpp"

namespace tria::asset::internal {

using RawData     = std::vector<char>;
using AssetLoader = AssetUnique (*)(AssetId id, const fs::path& path, RawData, DatabaseImpl*);

[[nodiscard]] auto loadAsset(AssetId id, const fs::path& path, RawData raw, DatabaseImpl* db)
    -> AssetUnique;

} // namespace tria::asset::internal

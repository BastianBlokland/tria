#pragma once
#include "../database_impl.hpp"
#include "tria/fs.hpp"

namespace tria::asset::internal {

using RawData = std::vector<char>;

[[nodiscard]] auto loadAsset(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData)
    -> AssetUnique;

} // namespace tria::asset::internal

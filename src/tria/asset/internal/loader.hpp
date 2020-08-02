#pragma once
#include "../database_impl.hpp"
#include "tria/fs.hpp"
#include "tria/math/pod_vector.hpp"

namespace tria::asset::internal {

[[nodiscard]] auto loadAsset(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, math::RawData)
    -> AssetUnique;

} // namespace tria::asset::internal

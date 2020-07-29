#include "loader.hpp"
#include <cassert>

namespace tria::asset::internal {

auto loadGraphic(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData) -> AssetUnique;
auto loadMeshObj(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData) -> AssetUnique;
auto loadImagePpm(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData) -> AssetUnique;
auto loadShader(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData) -> AssetUnique;
auto loadRawAsset(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData) -> AssetUnique;

namespace {

using AssetLoader = AssetUnique (*)(log::Logger*, DatabaseImpl*, AssetId, const fs::path&, RawData);

auto getLoader(const fs::path& path) -> AssetLoader {
  static const std::unordered_map<std::string, AssetLoader> table = {
      {".gfx", loadGraphic},
      {".obj", loadMeshObj},
      {".ppm", loadImagePpm},
      {".spv", loadShader},
  };
  auto itr = table.find(path.extension().string());
  if (itr != table.end()) {
    return itr->second;
  }
  // If no loader was found for the extension we fallback to the raw-asset loader.
  return loadRawAsset;
}

} // namespace

auto loadAsset(log::Logger* logger, DatabaseImpl* db, AssetId id, const fs::path& path, RawData raw)
    -> AssetUnique {
  assert(db);
  return getLoader(path)(logger, db, std::move(id), path, std::move(raw));
}

} // namespace tria::asset::internal

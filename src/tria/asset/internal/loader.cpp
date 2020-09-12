#include "loader.hpp"
#include <cassert>

namespace tria::asset::internal {

using RawData = math::RawData;

auto loadGraphic(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadMeshObj(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadFontTtf(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadTexturePpm(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadTextureTga(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadShaderSpv(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;
auto loadRawAsset(log::Logger*, DatabaseImpl*, AssetId, RawData) -> AssetUnique;

namespace {

using AssetLoader = AssetUnique (*)(log::Logger*, DatabaseImpl*, AssetId, RawData);

auto getLoader(const fs::path& path) -> AssetLoader {
  static const std::unordered_map<std::string, AssetLoader> table = {
      {".gfx", loadGraphic},
      {".obj", loadMeshObj},
      {".ttf", loadFontTtf},
      {".ppm", loadTexturePpm},
      {".tga", loadTextureTga},
      {".spv", loadShaderSpv},
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
  return getLoader(path)(logger, db, std::move(id), std::move(raw));
}

} // namespace tria::asset::internal

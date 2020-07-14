#include "loader.hpp"
#include <cassert>

namespace tria::asset::internal {

auto loadShader(AssetId, const fs::path&, RawData, DatabaseImpl*) -> AssetUnique;
auto loadRawAsset(AssetId, const fs::path&, RawData, DatabaseImpl*) -> AssetUnique;

namespace {

auto getLoader(const fs::path& path) -> AssetLoader {
  static const std::unordered_map<std::string, AssetLoader> table = {
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

auto loadAsset(AssetId id, const fs::path& path, RawData raw, DatabaseImpl* db) -> AssetUnique {
  assert(db);
  return getLoader(path)(std::move(id), path, std::move(raw), db);
}

} // namespace tria::asset::internal

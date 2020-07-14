#pragma once
#include "tria/asset/raw_asset.hpp"
#include "tria/fs.hpp"
#include "tria/pal/utils.hpp"
#include <fstream>
#include <string>

namespace tria::asset::tests {

inline auto writeFile(const fs::path& path, std::string data) {
  auto file = std::ofstream{path.string(), std::ios::out | std::ios::binary};
  file.write(data.data(), data.length());
}

template <typename TestFunc>
auto withTempDir(TestFunc func) {
  auto tmpDir = pal::getCurExecutablePath().parent_path() / "tria_asset_test";
  fs::create_directories(tmpDir);
  try {
    func(tmpDir);
    fs::remove_all(tmpDir);
  } catch (...) {
    fs::remove_all(tmpDir);
    throw;
  }
}

#define CHECK_RAW_ASSET(asset, expected)                                                           \
  do {                                                                                             \
    auto* assetPtr = asset;                                                                        \
    REQUIRE(assetPtr->getKind() == AssetKind::Raw);                                                \
    auto contentStr = std::string(                                                                 \
        assetPtr->downcast<RawAsset>()->getData(), assetPtr->downcast<RawAsset>()->getSize());     \
    auto expectedStr = std::string{expected};                                                      \
    CHECK(contentStr == expectedStr);                                                              \
  } while (false)

} // namespace tria::asset::tests

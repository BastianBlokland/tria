#pragma once
#include "tria/asset/raw_asset.hpp"
#include "tria/fs.hpp"
#include "tria/pal/utils.hpp"
#include <string>

namespace tria::asset::tests {

auto writeFile(const fs::path& path, std::string data) -> void;
auto writeFile(const fs::path& path, math::RawData data) -> void;
auto deleteDir(const fs::path& path) -> void;

template <typename TestFunc>
auto withTempDir(TestFunc func) {
  auto tmpDir = pal::getCurExecutablePath().parent_path() / "tria_asset_test";
  fs::create_directories(tmpDir);
  try {
    func(tmpDir);
    deleteDir(tmpDir);
  } catch (...) {
    deleteDir(tmpDir);
    throw;
  }
}

#define CHECK_RAW_ASSET(asset, expected)                                                           \
  do {                                                                                             \
    auto* assetPtr = asset;                                                                        \
    REQUIRE(assetPtr->getKind() == AssetKind::Raw);                                                \
    auto contentStr = std::string(                                                                 \
        reinterpret_cast<const char*>(assetPtr->downcast<RawAsset>()->getBegin()),                 \
        assetPtr->downcast<RawAsset>()->getSize());                                                \
    auto expectedStr = std::string{expected};                                                      \
    CHECK(contentStr == expectedStr);                                                              \
  } while (false)

} // namespace tria::asset::tests

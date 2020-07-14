#pragma once
#include "tria/asset/database.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace tria::asset {

using AssetUnique = std::unique_ptr<Asset>;

class DatabaseImpl final {
public:
  DatabaseImpl(log::Logger* logger, fs::path rootPath) :
      m_logger{logger}, m_rootPath{std::move(rootPath)} {}
  ~DatabaseImpl() = default;

  /* Get a pointer to an asset. Will either load it or return a previously loaded asset.
   *
   * TODO(bastian): In the future this should probably return a 'smart-pointer'-like handle so we
   * can track who is using it so we can decide when to unload it.
   *
   * Throws if asset loading fails.
   */
  [[nodiscard]] auto get(const AssetId& id) -> const Asset*;

private:
  log::Logger* m_logger;
  fs::path m_rootPath;

  std::mutex m_assetsMutex;
  std::unordered_map<AssetId, AssetUnique> m_assets;

  [[nodiscard]] auto getPath(const AssetId& id) const noexcept -> fs::path;
};

} // namespace tria::asset

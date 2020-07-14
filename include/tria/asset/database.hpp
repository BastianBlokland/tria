#pragma once
#include "tria/asset/asset.hpp"
#include "tria/fs.hpp"
#include "tria/log/api.hpp"

namespace tria::asset {

class DatabaseImpl;

/*
 * Database for loading assets from.
 * Assets are loaded lazily but cached for future requests.
 * Currently assets cannot be unloaded, in the future some ref-counting smart-pointer-like handle
 * should be returned to track asset usage.
 *
 * Api is threadsafe.
 */
class Database final {
public:
  Database() = delete;
  Database(log::Logger* logger, fs::path rootPath);
  Database(const Database& rhs)     = delete;
  Database(Database&& rhs) noexcept = default;
  ~Database();

  auto operator=(const Database& rhs) -> Database& = delete;
  auto operator=(Database&& rhs) noexcept -> Database& = default;

  /* Load an asset with a given id.
   * Is thread-safe.
   */
  [[nodiscard]] auto get(const AssetId& id) -> const Asset*;

private:
  std::unique_ptr<DatabaseImpl> m_impl;
};

} // namespace tria::asset

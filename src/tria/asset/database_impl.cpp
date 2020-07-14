#include "database_impl.hpp"
#include "internal/loader.hpp"
#include "tria/asset/asset.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include <cassert>
#include <chrono>
#include <fstream>

namespace tria::asset {

using RawData = std::vector<char>;
using Clock   = std::chrono::high_resolution_clock;

namespace {

auto loadRaw(const fs::path& path) -> RawData {
  auto file = std::ifstream{path.string(), std::ios::ate | std::ios::binary};
  if (!file.is_open()) {
    throw err::AssetLoadErr(path, "Failed to open file");
  }
  const auto fileSize = static_cast<size_t>(file.tellg());
  auto buffer         = std::vector<char>(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

} // namespace

auto DatabaseImpl::get(const AssetId& id) -> const Asset* {
  // Find if the asset has been already loaded, if so return a pointer to it.
  {
    const auto lk       = std::lock_guard<std::mutex>{m_assetsMutex};
    const auto assetItr = m_assets.find(id);
    if (assetItr != m_assets.end()) {
      return assetItr->second.get();
    }
  }

  auto loadBeginTime = Clock::now();

  const auto path = getPath(id);
  AssetUnique asset;
  try {
    auto rawData        = loadRaw(path);
    const auto dataSize = rawData.size();

    asset = internal::loadAsset(id, path, std::move(rawData), this);
    assert(asset);

    LOG_I(
        m_logger,
        "Asset loaded",
        {"id", id},
        {"path", path.string()},
        {"kind", getName(asset->getKind())},
        {"size", log::MemSize{dataSize}},
        {"duration", Clock::now() - loadBeginTime});

  } catch (const err::AssetLoadErr& err) {
    LOG_W(
        m_logger,
        "Failed to load asset",
        {"id", id},
        {"reason", err.getInnerMsg()},
        {"path", path.string()});
    throw;
  } catch (...) {
    LOG_W(m_logger, "Failed to load asset", {"id", id}, {"path", path.string()});
    throw;
  }

  // Save the asset in the map.
  {
    const auto lk = std::lock_guard<std::mutex>{m_assetsMutex};

    // Note: Another insertion might have beaten us to it, but in that case we will just return a
    // pointer to the previously inserted asset and discard our asset.
    const auto assetItr = m_assets.insert({id, std::move(asset)});
    return assetItr.first->second.get();
  }
}

auto DatabaseImpl::getPath(const AssetId& id) const noexcept -> fs::path { return m_rootPath / id; }

} // namespace tria::asset

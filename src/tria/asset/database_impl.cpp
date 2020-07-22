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

// Pad every file buffer with this amount of zero bytes at the end, parsers can rely on this fact
// and try to read atleast this amount 'past' the file.
constexpr size_t g_fileBufferPadding = 32;

// Maximum file size we support, guard against allocating huge amounts of memory.
constexpr size_t g_maxFileSize = 512 * 1024 * 1024;

auto loadRaw(const fs::path& path) -> RawData {
  if (!fs::is_regular_file(path)) {
    throw err::AssetLoadErr(path, "Path is not a file");
  }
  auto file = std::ifstream{path.string(), std::ios::ate | std::ios::binary};
  if (!file.is_open()) {
    throw err::AssetLoadErr(path, "Failed to open file");
  }
  const auto fileSize = file.tellg();
  if (fileSize < 0) {
    throw err::AssetLoadErr(path, "Failed to open file");
  }
  if (static_cast<size_t>(fileSize) > g_maxFileSize) {
    throw err::AssetLoadErr(path, "File too big");
  }
  auto buffer = std::vector<char>(static_cast<size_t>(fileSize) + g_fileBufferPadding);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  // Make the buffer size match the actual file size, note: this will not remove the zero padding we
  // added after the file.
  buffer.resize(fileSize);
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

    asset = internal::loadAsset(m_logger, this, id, path, std::move(rawData));
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

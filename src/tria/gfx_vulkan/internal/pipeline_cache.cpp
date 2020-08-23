#include "pipeline_cache.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/pal/utils.hpp"
#include "utils.hpp"
#include <fstream>
#include <optional>

namespace tria::gfx::internal {

namespace {

constexpr size_t g_maxPipelineCacheSize = 32U * 1024U * 1024U;
constexpr auto g_pipelineCacheExtension = "psoc"; // Pipeline State Object Cache.

[[nodiscard]] auto getPipelineCachePath() {
  return pal::getCurExecutablePath().replace_extension(g_pipelineCacheExtension);
}

/* Pipeline cache header.
 * See spec, table 12:
 * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VkPipelineCache
 */
struct CacheHeader final {
  uint32_t vendorId;
  uint32_t deviceId;
  uint8_t cacheId[VK_UUID_SIZE];
};

[[nodiscard]] auto
verifyCacheHeader(const CacheHeader& header, const VkPhysicalDeviceProperties& props) {
  return (
      header.vendorId == props.vendorID && header.deviceId == props.deviceID &&
      std::memcmp(header.cacheId, props.pipelineCacheUUID, VK_UUID_SIZE) == 0);
}

[[nodiscard]] auto loadFromDisk(const fs::path& path) -> std::optional<math::RawData> {
  auto file = std::ifstream{path.string(), std::ios::ate | std::ios::binary};
  if (!file) {
    return std::nullopt;
  }
  const auto fileSize = static_cast<size_t>(file.tellg());
  if (fileSize > g_maxPipelineCacheSize) {
    return std::nullopt;
  }

  auto buffer = math::RawData{fileSize};

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
  file.close();

  return std::optional<math::RawData>{std::move(buffer)};
}

/* Read a unsigned 32 bit integer as a string of bytes starting with the least significant byte.
 * Note: 4 bytes of data need to be available.
 */
[[nodiscard]] auto readUInt32(const uint8_t* data) -> uint32_t {
  return (
      (static_cast<uint32_t>(*(data + 0)) << 0U) | (static_cast<uint32_t>(*(data + 1)) << 8U) |
      (static_cast<uint32_t>(*(data + 2)) << 16U) | (static_cast<uint32_t>(*(data + 3)) << 24U));
}

[[nodiscard]] auto readCacheHeader(const math::RawData& buffer) -> std::optional<CacheHeader> {
  constexpr auto headerSize = 16U + VK_UUID_SIZE;

  if (buffer.size() < headerSize) {
    // Not big enough to contain a cache header.
    return std::nullopt;
  }
  if (readUInt32(buffer.begin()) != headerSize) {
    return std::nullopt;
  }
  const auto headerVersion = readUInt32(buffer.begin() + 4U);
  if (headerVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) {
    return std::nullopt;
  }

  CacheHeader result;
  result.vendorId = readUInt32(buffer.begin() + 8U);
  result.deviceId = readUInt32(buffer.begin() + 12U);
  for (auto i = 0U; i != VK_UUID_SIZE; ++i) {
    result.cacheId[i] = *(buffer.begin() + 16U + i);
  }
  return result;
}

[[nodiscard]] auto createVkPipelineCache(VkDevice vkDevice, const math::RawData* data)
    -> VkPipelineCache {
  VkPipelineCacheCreateInfo createInfo = {};
  createInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  createInfo.initialDataSize           = data ? data->size() : 0U;
  createInfo.pInitialData              = data ? data->data() : nullptr;

  VkPipelineCache result;
  checkVkResult(vkCreatePipelineCache(vkDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

[[nodiscard]] auto
loadVkPipelineCache(log::Logger* logger, VkDevice vkDevice, const VkPhysicalDeviceProperties& props)
    -> VkPipelineCache {

  const auto path    = getPipelineCachePath();
  const auto optFile = loadFromDisk(path);
  if (optFile) {
    const auto headerOpt = readCacheHeader(*optFile);
    if (headerOpt && verifyCacheHeader(*headerOpt, props)) {
      LOG_I(
          logger,
          "Loaded pipeline cache",
          {"path", path.string()},
          {"size", log::MemSize{optFile->size()}});
      return createVkPipelineCache(vkDevice, &optFile.value());
    }

    LOG_W(logger, "Invalid pipeline cache", {"path", path.string()});
  }

  LOG_I(logger, "Create new pipline cache");
  return createVkPipelineCache(vkDevice, nullptr);
}

auto saveVkPipelineCache(log::Logger* logger, VkDevice vkDevice, VkPipelineCache vkPipelineCache)
    -> void {

  // Query how much cache data is available.
  size_t size;
  vkGetPipelineCacheData(vkDevice, vkPipelineCache, &size, nullptr);
  size = std::min(size, g_maxPipelineCacheSize); // Limit the maximum cache size.

  // Allocate a buffer for it and write the cache data in it.
  auto buffer = math::RawData(size);
  vkGetPipelineCacheData(vkDevice, vkPipelineCache, &size, buffer.data());

  // Write the cache to disk.
  const auto path = getPipelineCachePath();
  auto file       = std::ofstream{path.string(), std::ios::out | std::ios::binary};
  if (file.good()) {
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();

    LOG_I(logger, "Saved pipeline cache", {"path", path.string()}, {"size", log::MemSize{size}});
    return;
  }

  LOG_W(
      logger,
      "Failed to save pipeline cache",
      {"path", path.string()},
      {"size", log::MemSize{size}});
}

} // namespace tria::gfx::internal

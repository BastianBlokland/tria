#pragma once
#include "tria/log/api.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>

namespace tria::gfx::internal {

class Device;

/* Repository for resources that are created per asset.
 */
template <typename T>
class AssetResource final {
public:
  using AssetType = typename T::AssetType;

  AssetResource(log::Logger* logger, Device* device) : m_logger{logger}, m_device{device} {}
  ~AssetResource() = default;

  template <typename... Parameters>
  [[nodiscard]] auto get(const AssetType* asset, Parameters&&... parameters) noexcept -> const T* {
    assert(asset);

    // If we already have a resource for the given asset then return that.
    const auto itr = m_data.find(asset);
    if (itr != m_data.end()) {
      return &itr->second;
    }

    // Otherwise construct a new resource.
    const auto insertItr = m_data.try_emplace(asset, m_logger, m_device, asset, parameters...);
    return &insertItr.first->second;
  }

private:
  log::Logger* m_logger;
  Device* m_device;
  std::unordered_map<const AssetType*, T> m_data;
};

template <typename T>
using AssetResourceUnique = std::unique_ptr<AssetResource<T>>;

} // namespace tria::gfx::internal

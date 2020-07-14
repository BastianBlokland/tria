#pragma once
#include "tria/asset/asset.hpp"
#include <vector>

namespace tria::asset {

/*
 * Assets where we have no special loader for, just contains the raw bytes.
 */
class RawAsset final : public Asset {
public:
  RawAsset(AssetId id, std::vector<char> data) :
      Asset{std::move(id), getKind()}, m_data{std::move(data)} {}
  RawAsset(const RawAsset& rhs) = delete;
  RawAsset(RawAsset&& rhs)      = delete;
  ~RawAsset() noexcept          = default;

  auto operator=(const RawAsset& rhs) -> RawAsset& = delete;
  auto operator=(RawAsset&& rhs) -> RawAsset& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Raw; }

  [[nodiscard]] auto getSize() const noexcept { return m_data.size(); }
  [[nodiscard]] auto getData() const noexcept -> const char* { return m_data.data(); }

private:
  std::vector<char> m_data;
};

} // namespace tria::asset

#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"

namespace tria::asset {

/*
 * Assets where we have no special loader for, just contains the raw bytes.
 */
class RawAsset final : public Asset {
public:
  RawAsset(AssetId id, math::RawData data) :
      Asset{std::move(id), getKind()}, m_data{std::move(data)} {}
  RawAsset(const RawAsset& rhs) = delete;
  RawAsset(RawAsset&& rhs)      = delete;
  ~RawAsset() noexcept          = default;

  auto operator=(const RawAsset& rhs) -> RawAsset& = delete;
  auto operator=(RawAsset&& rhs) -> RawAsset& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Raw; }

  [[nodiscard]] auto getSize() const noexcept { return m_data.size(); }
  [[nodiscard]] auto getBegin() const noexcept { return m_data.begin(); }
  [[nodiscard]] auto getEnd() const noexcept { return m_data.end(); }

private:
  math::RawData m_data;
};

} // namespace tria::asset

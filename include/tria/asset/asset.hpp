#pragma once
#include "tria/asset/asset_kind.hpp"
#include "tria/asset/err/asset_type_err.hpp"
#include <string>

namespace tria::asset {

using AssetId = std::string;

/*
 * Abstract base class for asset implementations.
 */
class Asset {
public:
  Asset()                     = delete;
  Asset(const Asset& rhs)     = delete;
  Asset(Asset&& rhs) noexcept = delete;
  virtual ~Asset()            = default;

  auto operator=(const Asset& rhs) -> Asset& = delete;
  auto operator=(Asset&& rhs) noexcept -> Asset& = delete;

  [[nodiscard]] auto getId() const noexcept -> const AssetId& { return m_id; }
  [[nodiscard]] auto getKind() const noexcept { return m_kind; }

  template <typename AssetType>
  [[nodiscard]] auto downcast() const -> const AssetType* {
    if (getKind() != AssetType::getKind()) {
      throw err::AssetTypeErr{"Invalid downcast"};
    }
    return static_cast<const AssetType*>(this);
  }

protected:
  Asset(AssetId id, AssetKind kind) noexcept : m_id{std::move(id)}, m_kind{kind} {}

private:
  AssetId m_id;
  AssetKind m_kind;
};

} // namespace tria::asset

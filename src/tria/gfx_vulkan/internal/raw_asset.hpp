#pragma once
#include "tria/fs.hpp"
#include <memory>
#include <vector>

namespace tria::gfx::internal {

class RawAsset final {
public:
  RawAsset(std::vector<char> data) : m_data{std::move(data)} {}

  [[nodiscard]] auto begin() { return m_data.begin(); }
  [[nodiscard]] auto end() { return m_data.end(); }

  [[nodiscard]] auto getSize() const noexcept { return m_data.size(); }
  [[nodiscard]] auto getData() const noexcept -> const char* { return m_data.data(); }

private:
  std::vector<char> m_data;
};

using RawAssetPtr = std::unique_ptr<RawAsset>;

[[nodiscard]] auto loadRawAsset(const fs::path& path) -> RawAssetPtr;

} // namespace tria::gfx::internal

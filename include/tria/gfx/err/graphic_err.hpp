#pragma once
#include "tria/asset/asset.hpp"
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string>
#include <string_view>

namespace tria::gfx::err {

/*
 * Exception that is thrown when an invalid graphic setup is encountered.
 */
class GraphicErr final : public std::exception {
public:
  GraphicErr() = delete;
  GraphicErr(asset::AssetId assetId, std::string_view msg) : m_assetId{std::move(assetId)} {
    char buffer[128];
    std::snprintf(buffer, sizeof(buffer), "Graphic error (asset: %s): ", m_assetId.c_str());
    m_msg += buffer;
    m_msg += msg;
  }

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

  [[nodiscard]] auto getAssetId() const noexcept -> const asset::AssetId& { return m_assetId; }

private:
  asset::AssetId m_assetId;
  std::string m_msg;
};

} // namespace tria::gfx::err

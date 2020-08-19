#pragma once
#include "tria/asset/asset.hpp"
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string>
#include <string_view>

namespace tria::gfx::err {

/*
 * Exception that is thrown when an invalid shader setup is encountered.
 */
class ShaderErr final : public std::exception {
public:
  ShaderErr() = delete;
  ShaderErr(uint32_t set, uint32_t binding, asset::AssetId assetId, std::string_view msg) :
      m_set{set}, m_binding{binding}, m_assetId{std::move(assetId)} {
    char buffer[128];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "Shader error (shader: %s, set: '%u', binding: '%u'): ",
        m_assetId.c_str(),
        m_set,
        m_binding);
    m_msg += buffer;
    m_msg += msg;
  }

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

  [[nodiscard]] auto getSet() const noexcept { return m_set; }
  [[nodiscard]] auto getBinding() const noexcept { return m_binding; }
  [[nodiscard]] auto getAssetId() const noexcept -> const asset::AssetId& { return m_assetId; }

private:
  uint32_t m_set;
  uint32_t m_binding;
  asset::AssetId m_assetId;
  std::string m_msg;
};

} // namespace tria::gfx::err

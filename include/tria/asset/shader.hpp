#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"

namespace tria::asset {

enum class ShaderKind : uint8_t {
  SpvVertex   = 1,
  SpvFragment = 2,
};

/*
 * Asset containing shader code.
 * Note: only contains the raw shader code, this asset is not reposible for uploading to the gpu.
 */
class Shader final : public Asset {
public:
  Shader(AssetId id, ShaderKind shaderKind, math::RawData data) :
      Asset{std::move(id), getKind()}, m_shaderKind{shaderKind}, m_data{std::move(data)} {}
  Shader(const Shader& rhs) = delete;
  Shader(Shader&& rhs)      = delete;
  ~Shader() noexcept        = default;

  auto operator=(const Shader& rhs) -> Shader& = delete;
  auto operator=(Shader&& rhs) -> Shader& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Shader; }

  [[nodiscard]] auto getShaderKind() const noexcept { return m_shaderKind; }

  [[nodiscard]] auto getSize() const noexcept { return m_data.size(); }
  [[nodiscard]] auto getBegin() const noexcept { return m_data.begin(); }
  [[nodiscard]] auto getEnd() const noexcept { return m_data.end(); }

private:
  ShaderKind m_shaderKind;
  math::RawData m_data;
};

} // namespace tria::asset

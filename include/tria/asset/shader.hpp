#pragma once
#include "tria/asset/asset.hpp"
#include "tria/math/pod_vector.hpp"
#include <string_view>
#include <vector>

namespace tria::asset {

/* Type of the shader, aka the 'stage'.
 */
enum class ShaderKind : uint8_t {
  SpvVertex   = 1,
  SpvFragment = 2,
};

/* Type of a shader input resource.
 */
enum class ShaderResourceKind : uint8_t {
  Texture       = 1,
  UniformBuffer = 2,
  StorageBuffer = 3,
};

/* Upper bounds on shader set and binding.
 */
constexpr auto maxShaderSets     = 32U; // Sets 0 - 31 are valid.
constexpr auto maxShaderBindings = 32U; // Bindings 0 - 31 are valid.

/* Shader input resource.
 * Set and binding are used by the graphics driver to link a resource to a shader.
 */
class ShaderResource final {
public:
  ShaderResource() = delete;
  ShaderResource(ShaderResourceKind kind, uint32_t set, uint32_t binding) :
      m_kind{kind}, m_set{set}, m_binding{binding} {}

  [[nodiscard]] constexpr auto operator==(const ShaderResource& rhs) const noexcept -> bool {
    return m_kind == rhs.m_kind && m_set == rhs.m_set && m_binding == rhs.m_binding;
  }

  [[nodiscard]] constexpr auto operator!=(const ShaderResource& rhs) const noexcept -> bool {
    return !operator==(rhs);
  }

  [[nodiscard]] auto getKind() const noexcept { return m_kind; }
  [[nodiscard]] auto getSet() const noexcept { return m_set; }
  [[nodiscard]] auto getBinding() const noexcept { return m_binding; }

private:
  ShaderResourceKind m_kind;
  uint32_t m_set;
  uint32_t m_binding;
};

/*
 * Asset containing shader code.
 * Note: only contains the raw shader code, this asset is not reposible for uploading to the gpu.
 */
class Shader final : public Asset {
public:
  Shader(
      AssetId id,
      ShaderKind shaderKind,
      std::string entryPointName,
      std::vector<ShaderResource> resources,
      math::RawData data) :
      Asset{std::move(id), getKind()},
      m_shaderKind{shaderKind},
      m_entryPointName{std::move(entryPointName)},
      m_resources{resources},
      m_data{std::move(data)} {}
  Shader(const Shader& rhs) = delete;
  Shader(Shader&& rhs)      = delete;
  ~Shader() noexcept        = default;

  auto operator=(const Shader& rhs) -> Shader& = delete;
  auto operator=(Shader&& rhs) -> Shader& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Shader; }

  [[nodiscard]] auto getShaderKind() const noexcept { return m_shaderKind; }
  [[nodiscard]] auto getEntryPointName() const noexcept -> std::string_view {
    return m_entryPointName;
  }

  [[nodiscard]] auto getResourceCount() const noexcept { return m_resources.size(); }
  [[nodiscard]] auto getResourceBegin() const noexcept -> const ShaderResource* {
    return m_resources.data();
  }
  [[nodiscard]] auto getResourceEnd() const noexcept -> const ShaderResource* {
    return m_resources.data() + m_resources.size();
  }

  [[nodiscard]] auto getSize() const noexcept { return m_data.size(); }
  [[nodiscard]] auto getBegin() const noexcept { return m_data.begin(); }
  [[nodiscard]] auto getEnd() const noexcept { return m_data.end(); }

private:
  ShaderKind m_shaderKind;
  std::string m_entryPointName;
  std::vector<ShaderResource> m_resources;
  math::RawData m_data;
};

} // namespace tria::asset

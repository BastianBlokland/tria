#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/shader.hpp"
#include <optional>
#include <string_view>

namespace tria::asset::internal {

namespace {

auto endsWith(const std::string& str, const std::string_view suffix) noexcept {
  return str.size() >= suffix.size() &&
      str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

[[nodiscard]] auto getShaderKind(const fs::path& path) noexcept -> std::optional<ShaderKind> {
  auto str = path.string();
  if (endsWith(str, ".vert.spv")) {
    return ShaderKind::SpvVertex;
  }
  if (endsWith(str, ".frag.spv")) {
    return ShaderKind::SpvFragment;
  }
  return std::nullopt;
}

} // namespace

auto loadShader(
    log::Logger* /*unused*/,
    DatabaseImpl* /*unused*/,
    AssetId id,
    const fs::path& path,
    math::RawData raw) -> AssetUnique {

  auto shaderKind = getShaderKind(path);
  if (!shaderKind) {
    throw err::AssetLoadErr{path, "Unable to determine shader type"};
  }
  return std::make_unique<Shader>(std::move(id), *shaderKind, std::move(raw));
}

} // namespace tria::asset::internal

#include "json.hpp"
#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/graphic.hpp"
#include <optional>
#include <string_view>
#include <unordered_map>

namespace tria::asset::internal {

namespace {

[[nodiscard]] auto getTextureFilterMode(std::string_view str) -> std::optional<FilterMode> {
  static const std::unordered_map<std::string_view, FilterMode> table = {
      {"nearest", FilterMode::Nearest},
      {"linear", FilterMode::Linear},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getBlendMode(std::string_view str) -> std::optional<BlendMode> {
  static const std::unordered_map<std::string_view, BlendMode> table = {
      {"none", BlendMode::None},
      {"alpha", BlendMode::Alpha},
      {"additive", BlendMode::Additive},
      {"alphaAdditive", BlendMode::AlphaAdditive},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getDepthTestMode(std::string_view str) -> std::optional<DepthTestMode> {
  static const std::unordered_map<std::string_view, DepthTestMode> table = {
      {"none", DepthTestMode::None},
      {"less", DepthTestMode::Less},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

} // namespace

auto loadGraphic(
    log::Logger* /*unused*/, DatabaseImpl* db, AssetId id, const fs::path& path, math::RawData raw)
    -> AssetUnique {

  simdjson::dom::object obj;
  auto err = parseJson(raw).get(obj);
  if (err) {
    throw err::AssetLoadErr{path, error_message(err)};
  }

  // Vertex shader.
  std::string_view vertShaderId;
  if (obj.at("vertShader").get(vertShaderId)) {
    throw err::AssetLoadErr{path, "No 'vertShader' field found on graphic"};
  }
  auto* vertShader = db->get(AssetId{vertShaderId})->downcast<Shader>();
  if (vertShader->getShaderKind() != ShaderKind::SpvVertex) {
    throw err::AssetLoadErr{path, "Invalid vertex shader"};
  }

  // Fragment shader.
  std::string_view fragShaderId;
  if (obj.at("fragShader").get(fragShaderId)) {
    throw err::AssetLoadErr{path, "No 'fragShader' field found on graphic"};
  }
  auto* fragShader = db->get(AssetId{fragShaderId})->downcast<Shader>();
  if (fragShader->getShaderKind() != ShaderKind::SpvFragment) {
    throw err::AssetLoadErr{path, "Invalid fragment shader"};
  }

  // Mesh.
  std::string_view meshId;
  if (obj.at("mesh").get(meshId)) {
    throw err::AssetLoadErr{path, "No 'mesh' field found on graphic"};
  }
  auto* mesh = db->get(AssetId{meshId})->downcast<Mesh>();

  // Samplers (optional field).
  auto samplers = std::vector<TextureSampler>{};
  simdjson::dom::array samplersArray;
  if (!obj.at("samplers").get(samplersArray)) {

    for (const auto& elem : samplersArray) {

      // Texture.
      std::string_view textureId;
      if (elem.at("texture").get(textureId)) {
        throw err::AssetLoadErr{path, "Object in sampler array is missing a 'texture' field"};
      }
      const auto* texture = db->get(AssetId{textureId})->downcast<Texture>();

      // Filter mode (optional field).
      auto filterMode = FilterMode::Linear;
      std::string_view filterStr;
      if (!elem.at("filter").get(filterStr)) {
        auto filterModeOpt = getTextureFilterMode(filterStr);
        if (!filterModeOpt) {
          throw err::AssetLoadErr{path, "Unsupported filter mode"};
        }
        filterMode = *filterModeOpt;
      }

      samplers.push_back(TextureSampler{texture, filterMode});
    }
  }

  // Blend mode (optional field).
  auto blendMode = BlendMode::None;
  std::string_view blendStr;
  if (!obj.at("blend").get(blendStr)) {
    auto blendModeOpt = getBlendMode(blendStr);
    if (!blendModeOpt) {
      throw err::AssetLoadErr{path, "Unsupported blend mode"};
    }
    blendMode = *blendModeOpt;
  }

  // Depth test mode (optional field).
  auto depthTestMode = DepthTestMode::None;
  std::string_view depthTestStr;
  if (!obj.at("depthTest").get(depthTestStr)) {
    auto depthTestModeOpt = getDepthTestMode(depthTestStr);
    if (!depthTestModeOpt) {
      throw err::AssetLoadErr{path, "Unsupported depth-test mode"};
    }
    depthTestMode = *depthTestModeOpt;
  }

  return std::make_unique<Graphic>(
      std::move(id), vertShader, fragShader, mesh, std::move(samplers), blendMode, depthTestMode);
}

} // namespace tria::asset::internal

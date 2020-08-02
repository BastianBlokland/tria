#include "json.hpp"
#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/graphic.hpp"
#include <optional>
#include <string_view>
#include <unordered_map>

namespace tria::asset::internal {

namespace {

using TexFilterMode = TextureSampler::FilterMode;

[[nodiscard]] auto getTextureFilterMode(std::string_view str) -> std::optional<TexFilterMode> {
  static const std::unordered_map<std::string_view, TexFilterMode> table = {
      {"nearest", TexFilterMode::Nearest},
      {"linear", TexFilterMode::Linear},
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

      // Filter mode.
      auto filterMode = TexFilterMode::Linear;
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

  return std::make_unique<Graphic>(
      std::move(id), vertShader, fragShader, mesh, std::move(samplers));
}

} // namespace tria::asset::internal

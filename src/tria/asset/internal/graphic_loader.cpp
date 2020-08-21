#include "json.hpp"
#include "loader.hpp"
#include "tria/asset/err/graphic_err.hpp"
#include "tria/asset/err/json_err.hpp"
#include "tria/asset/graphic.hpp"
#include <optional>
#include <string_view>
#include <unordered_map>

namespace tria::asset::internal {

namespace {

[[nodiscard]] auto getVertexTopology(std::string_view str) -> std::optional<VertexTopology> {
  static const std::unordered_map<std::string_view, VertexTopology> table = {
      {"triangles", VertexTopology::Triangles},
      {"lines", VertexTopology::Lines},
      {"linestrip", VertexTopology::LineStrip},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getRasterizerMode(std::string_view str) -> std::optional<RasterizerMode> {
  static const std::unordered_map<std::string_view, RasterizerMode> table = {
      {"fill", RasterizerMode::Fill},
      {"lines", RasterizerMode::Lines},
      {"points", RasterizerMode::Points},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getTextureFilterMode(std::string_view str) -> std::optional<FilterMode> {
  static const std::unordered_map<std::string_view, FilterMode> table = {
      {"nearest", FilterMode::Nearest},
      {"linear", FilterMode::Linear},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getTextureAnisotropyMode(std::string_view str) -> std::optional<AnisotropyMode> {
  static const std::unordered_map<std::string_view, AnisotropyMode> table = {
      {"none", AnisotropyMode::None},
      {"x2", AnisotropyMode::X2},
      {"x4", AnisotropyMode::X4},
      {"x8", AnisotropyMode::X8},
      {"x16", AnisotropyMode::X16},
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
      {"always", DepthTestMode::Always},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

[[nodiscard]] auto getCullMode(std::string_view str) -> std::optional<CullMode> {
  static const std::unordered_map<std::string_view, CullMode> table = {
      {"none", CullMode::None},
      {"back", CullMode::Back},
      {"front", CullMode::Front},
  };
  const auto search = table.find(str);
  return search == table.end() ? std::nullopt : std::optional{search->second};
}

} // namespace

auto loadGraphic(log::Logger* /*unused*/, DatabaseImpl* db, AssetId id, math::RawData raw)
    -> AssetUnique {

  simdjson::dom::object obj;
  auto err = parseJson(raw).get(obj);
  if (err) {
    throw err::JsonErr{error_message(err)};
  }

  // Shaders.
  auto shaders = std::vector<const Shader*>{};
  simdjson::dom::array shadersArray;
  if (!obj.at("shaders").get(shadersArray)) {
    for (const auto& elem : shadersArray) {
      std::string_view shaderId;
      if (elem.get(shaderId)) {
        throw err::GraphicErr{"Invalid shader reference"};
      }
      shaders.push_back(db->get(AssetId{shaderId})->downcast<Shader>());
    }
  }
  // Require exactly one vertex and one fragment shader at the moment.
  if (std::count_if(shaders.begin(), shaders.end(), [](const auto* shader) {
        return shader->getShaderKind() == ShaderKind::SpvVertex;
      }) != 1U) {
    throw err::GraphicErr{"Incorrect vertex shader count, expected 1"};
  }
  if (std::count_if(shaders.begin(), shaders.end(), [](const auto* shader) {
        return shader->getShaderKind() == ShaderKind::SpvFragment;
      }) != 1U) {
    throw err::GraphicErr{"Incorrect fragment shader count, expected 1"};
  }

  // Mesh  (optional field).
  const Mesh* mesh = nullptr;
  std::string_view meshId;
  if (!obj.at("mesh").get(meshId)) {
    mesh = db->get(AssetId{meshId})->downcast<Mesh>();
  }

  // Samplers (optional field).
  auto samplers = std::vector<TextureSampler>{};
  simdjson::dom::array samplersArray;
  if (!obj.at("samplers").get(samplersArray)) {

    for (const auto& elem : samplersArray) {

      // Texture.
      std::string_view textureId;
      if (elem.at("texture").get(textureId)) {
        throw err::GraphicErr{"Object in sampler array is missing a 'texture' field"};
      }
      const auto* texture = db->get(AssetId{textureId})->downcast<Texture>();

      // Filter mode (optional field).
      auto filterMode = FilterMode::Linear;
      std::string_view filterStr;
      if (!elem.at("filter").get(filterStr)) {
        auto filterModeOpt = getTextureFilterMode(filterStr);
        if (!filterModeOpt) {
          throw err::GraphicErr{"Unsupported filter mode"};
        }
        filterMode = *filterModeOpt;
      }

      // Anisotropy mode (optional field).
      auto anisoMode = AnisotropyMode::None;
      std::string_view anisoModeStr;
      if (!elem.at("anisotropy").get(anisoModeStr)) {
        auto anisoModeOpt = getTextureAnisotropyMode(anisoModeStr);
        if (!anisoModeOpt) {
          throw err::GraphicErr{"Unsupported anisotropy filter mode"};
        }
        anisoMode = *anisoModeOpt;
      }

      samplers.push_back(TextureSampler{texture, filterMode, anisoMode});
    }
  }

  // Vertex topology (optional field).
  auto vertexTopology = VertexTopology::Triangles;
  std::string_view topologyStr;
  if (!obj.at("topology").get(topologyStr)) {
    auto vertexTopologyOpt = getVertexTopology(topologyStr);
    if (!vertexTopologyOpt) {
      throw err::GraphicErr{"Unsupported vertex topology"};
    }
    vertexTopology = *vertexTopologyOpt;
  }

  // Rasterizer mode (optional field).
  auto rasterizerMode = RasterizerMode::Fill;
  std::string_view rasterizerStr;
  if (!obj.at("rasterizer").get(rasterizerStr)) {
    auto rasterizerModeOpt = getRasterizerMode(rasterizerStr);
    if (!rasterizerModeOpt) {
      throw err::GraphicErr{"Unsupported rasterizer mode"};
    }
    rasterizerMode = *rasterizerModeOpt;
  }

  // Line width (optional field).
  double lineWidth;
  if (obj.at("lineWidth").get(lineWidth)) {
    lineWidth = 1.; // Default to 1 pixel wide lines.
  }

  // Blend mode (optional field).
  auto blendMode = BlendMode::None;
  std::string_view blendStr;
  if (!obj.at("blend").get(blendStr)) {
    auto blendModeOpt = getBlendMode(blendStr);
    if (!blendModeOpt) {
      throw err::GraphicErr{"Unsupported blend mode"};
    }
    blendMode = *blendModeOpt;
  }

  // Depth test mode (optional field).
  auto depthTestMode = DepthTestMode::None;
  std::string_view depthTestStr;
  if (!obj.at("depthTest").get(depthTestStr)) {
    auto depthTestModeOpt = getDepthTestMode(depthTestStr);
    if (!depthTestModeOpt) {
      throw err::GraphicErr{"Unsupported depth-test mode"};
    }
    depthTestMode = *depthTestModeOpt;
  }

  // Cull mode (optional field).
  auto cullMode = CullMode::Back;
  std::string_view cullModeStr;
  if (!obj.at("cull").get(cullModeStr)) {
    auto cullModeOpt = getCullMode(cullModeStr);
    if (!cullModeOpt) {
      throw err::GraphicErr{"Unsupported cull mode"};
    }
    cullMode = *cullModeOpt;
  }

  return std::make_unique<Graphic>(
      std::move(id),
      std::move(shaders),
      mesh,
      std::move(samplers),
      vertexTopology,
      rasterizerMode,
      static_cast<float>(lineWidth),
      blendMode,
      depthTestMode,
      cullMode);
}

} // namespace tria::asset::internal

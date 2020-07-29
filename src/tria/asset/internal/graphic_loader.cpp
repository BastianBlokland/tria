#include "json.hpp"
#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/graphic.hpp"
#include <string_view>

namespace tria::asset::internal {

auto loadGraphic(
    log::Logger* /*unused*/, DatabaseImpl* db, AssetId id, const fs::path& path, RawData raw)
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

  // Images (optional field).
  auto images = std::vector<const Image*>{};
  simdjson::dom::array imageArray;
  if (!obj.at("images").get(imageArray)) {
    std::string_view imageId;
    for (const auto& elem : imageArray) {
      if (elem.get(imageId)) {
        throw err::AssetLoadErr{path, "Image array contains non-string element"};
      }
      images.push_back(db->get(AssetId{imageId})->downcast<Image>());
    }
  }

  return std::make_unique<Graphic>(std::move(id), vertShader, fragShader, mesh, std::move(images));
}

} // namespace tria::asset::internal

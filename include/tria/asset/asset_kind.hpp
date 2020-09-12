#pragma once
#include <cstdint>
#include <string_view>

namespace tria::asset {

enum class AssetKind : uint8_t {
  Raw     = 1,
  Mesh    = 2,
  Texture = 3,
  Font    = 4,
  Shader  = 5,
  Graphic = 6,
};

[[nodiscard]] constexpr auto getName(AssetKind kind) noexcept -> std::string_view {
  switch (kind) {
  case AssetKind::Raw:
    return "raw";
  case AssetKind::Mesh:
    return "mesh";
  case AssetKind::Texture:
    return "texture";
  case AssetKind::Font:
    return "font";
  case AssetKind::Shader:
    return "shader";
  case AssetKind::Graphic:
    return "graphic";
  }
  return "unkown";
}

} // namespace tria::asset

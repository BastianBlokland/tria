#pragma once
#include <cstdint>
#include <string_view>

namespace tria::asset {

enum class AssetKind : uint8_t {
  Raw     = 1,
  Shader  = 2,
  Graphic = 3,
};

[[nodiscard]] constexpr auto getName(AssetKind kind) noexcept -> std::string_view {
  switch (kind) {
  case AssetKind::Raw:
    return "raw";
  case AssetKind::Shader:
    return "shader";
  case AssetKind::Graphic:
    return "graphic";
  }
  return "unkown";
}

} // namespace tria::asset

#include "loader.hpp"
#include "tiny_obj_loader.h"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec.hpp"
#include <istream>
#include <streambuf>

namespace tria::asset::internal {

namespace {

class MemBuff : public std::streambuf {
public:
  MemBuff(char* data, size_t size) { this->setg(data, data, data + size); }
};

/* Adapter to feed a raw buffer to a function taking an std::istream.
 */
class IMemStream final : MemBuff, public std::istream {
public:
  IMemStream(char* data, size_t size) :
      MemBuff(data, size), std::istream(static_cast<std::streambuf*>(this)) {}
};

} // namespace

auto loadMeshObj(
    log::Logger* logger, DatabaseImpl* /*unused*/, AssetId id, const fs::path& path, RawData raw)
    -> AssetUnique {

  auto input     = IMemStream{raw.data(), raw.size()};
  auto attrib    = tinyobj::attrib_t{};
  auto shapes    = std::vector<tinyobj::shape_t>{};
  auto materials = std::vector<tinyobj::material_t>{};
  auto warn      = std::string{};
  auto err       = std::string{};
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &input)) {
    throw err::AssetLoadErr{path, err};
  }

  if (!warn.empty()) {
    LOG_W(logger, "Obj import warning", {"message", warn}, {"id", id}, {"path", path.string()});
  }

  if (shapes.empty()) {
    throw err::AssetLoadErr{path, "No shape found in obj"};
  }

  auto vertices = std::vector<Vertex>{};
  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      auto pos = math::Vec3f{
          attrib.vertices[index.vertex_index * 3 + 0],
          attrib.vertices[index.vertex_index * 3 + 1],
          attrib.vertices[index.vertex_index * 3 + 2]};
      auto col = math::Color{
          attrib.colors[index.vertex_index * 3 + 0],
          attrib.colors[index.vertex_index * 3 + 1],
          attrib.colors[index.vertex_index * 3 + 2],
          1.0f};
      vertices.emplace_back(pos, col);
    }
  }
  return std::make_unique<Mesh>(std::move(id), std::move(vertices));
}

} // namespace tria::asset::internal

#include "loader.hpp"
#include "mesh_builder.hpp"
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
  MemBuff(uint8_t* data, size_t size) {
    this->setg(
        reinterpret_cast<char*>(data),
        reinterpret_cast<char*>(data),
        reinterpret_cast<char*>(data) + size);
  }
};

/* Adapter to feed a raw buffer to a function taking an std::istream.
 */
class IMemStream final : MemBuff, public std::istream {
public:
  IMemStream(uint8_t* data, size_t size) :
      MemBuff(data, size), std::istream(static_cast<std::streambuf*>(this)) {}
};

} // namespace

auto loadMeshObj(
    log::Logger* logger,
    DatabaseImpl* /*unused*/,
    AssetId id,
    const fs::path& path,
    math::RawData raw) -> AssetUnique {

  auto input     = IMemStream{raw.begin(), raw.size()};
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

  auto vertices    = math::PodVector<Vertex>{};
  auto indices     = math::PodVector<IndexType>{};
  auto meshBuilder = MeshBuilder{&vertices, &indices};

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

      auto texcoord = index.texcoord_index < 0
          ? math::Vec2f{}
          : math::Vec2f{
                attrib.texcoords[index.texcoord_index * 2 + 0],
                1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]};
      meshBuilder.pushVertex(Vertex{pos, col, texcoord});
    }
  }
  return std::make_unique<Mesh>(std::move(id), std::move(vertices), std::move(indices));
}

} // namespace tria::asset::internal

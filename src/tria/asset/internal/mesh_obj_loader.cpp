#include "loader.hpp"
#include "mesh_builder.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec.hpp"
#include <limits>

namespace tria::asset::internal {

/* Wavefront Obj.
 * Only polygonal faces are supported (no curves or lines), materials are also ignored at this time.
 * Format specification: http://www.martinreddy.net/gfx/3d/OBJ.spec
 * Faces are assumed to be contex and a triangulated using a simple triangle fan.
 */

namespace {

constexpr auto g_unusedVertexElementSentinel = std::numeric_limits<int>::max();

class Reader final {
public:
  Reader(const uint8_t* current) : m_cur{current} {}

  [[nodiscard]] auto getCur() noexcept -> const uint8_t*& { return m_cur; }

  /* Read a single character.
   * Last character is guaranteed to be a null-terminator, reading beyond the null-terminator is
   * undefined behaviour.
   */
  auto consumeChar() noexcept -> char { return *m_cur++; }

  /* Read a character if it matches the given character.
   */
  auto consumeChar(char c) noexcept -> bool {
    if (*m_cur == c) {
      ++m_cur;
      return true;
    }
    return false;
  }

  /* Read a single unsigned int.
   */
  auto consumeUInt() noexcept -> unsigned int {
    auto result = 0U;
    while (true) {
      const auto c = *m_cur;
      switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        result *= 10U;
        result += c - '0';
        ++m_cur;
        break;
      default:
        return result;
      }
    }
  }

  /* Read a single signed int.
   */
  auto consumeInt() noexcept -> int {
    if (consumeChar('-')) {
      return -static_cast<int>(consumeUInt());
    }
    return static_cast<int>(consumeUInt());
  }

  /* Read a single float.
   */
  auto consumeFloat() noexcept -> float {
    auto sign              = consumeChar('-') ? -1.f : 1.f;
    auto result            = 0.f;
    auto divider           = 1.f;
    auto dividerMultiplier = 1.f;
    while (true) {
      const auto c = *m_cur;
      switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        result = result * 10.f + (c - '0');
        divider *= dividerMultiplier;
        ++m_cur;
        break;
      case '.':
        dividerMultiplier = 10.f;
        ++m_cur;
        break;
      case 'e':
      case 'E': {
        // Obj saids nothing about supporting scientific notation, but there are files in the wild
        // that use it.
        consumeChar();
        divider /= std::pow(10.f, consumeInt());
        goto End;
      }
      default:
        goto End;
      }
    }
  End:
    return result / divider * sign;
  }

  /* Does NOT consume newlines.
   */
  auto consumeWhitespace() noexcept {
    while (true) {
      switch (*m_cur) {
      case ' ':
      case '\t':
      case 0x0B:
      case 0x0C:
        // Ascii whitespace characters: ignore.
        ++m_cur;
        break;
      default:
        return;
      }
    }
  }

  /* Consume the rest of the line including the newline character.
   */
  auto consumeRestOfLine() noexcept {
    while (true) {
      switch (*m_cur) {
      case '\n':
        ++m_cur;
        return;
      case '\0':
        return;
      default:
        ++m_cur;
        break;
      }
    }
  }

private:
  const uint8_t* m_cur;
};

/* Indices for a single face vertex.
 * These are already bounds checked and converted to absolute indices starting from 0.
 * Normal and texcoord are optional, 'g_unusedVertexElementSentinel' means unused.
 */
struct ObjVertex final {
  int positionIndex;
  int normalIndex;
  int texcoordIndex;
};

/* Obj face.
 * Contains three or more vertices, no upper bound on amount of vertices.
 */
struct ObjFace final {
  unsigned int vertexIndex;
  unsigned int vertexCount;
  bool useFaceNormal; // Indicates that surface normal should be used instead of per vertex, happens
                      // if not all vertices define a vertex normal.
};

struct ObjData final {
  math::PodVector<math::Vec3f> positions;
  math::PodVector<math::Vec2f> texcoords;
  math::PodVector<math::Vec3f> normals;
  math::PodVector<ObjVertex> vertices;
  math::PodVector<ObjFace> faces;
};

/* Read x and y floats seperated by whitespace.
 * The y component is inverted.
 */
auto readVec2InvertY(Reader& reader) noexcept -> math::Vec2f {
  math::Vec2f result;
  result.x() = reader.consumeFloat();
  reader.consumeWhitespace();
  result.y() = 1.f - reader.consumeFloat();
  return result;
}

/* Read x, y and z floats seperated by whitespace.
 */
auto readVec3(Reader& reader) noexcept -> math::Vec3f {
  math::Vec3f result;
  result.x() = reader.consumeFloat();
  reader.consumeWhitespace();
  result.y() = reader.consumeFloat();
  reader.consumeWhitespace();
  result.z() = reader.consumeFloat();
  return result;
}

/* Read a obj vertex definition.
 * position index / texcoord index / normal index.
 * Obj indices are 1 based, we convert them to be zero based.
 * Negative indices can be used to index relative to the end of the current data.
 */
auto readObjVertex(Reader& reader, const ObjData& d) -> ObjVertex {
  ObjVertex res;
  res.texcoordIndex = g_unusedVertexElementSentinel; // Optional element.
  res.normalIndex   = g_unusedVertexElementSentinel; // Optional element.

  // Position index (optionally prefixed by 'v').
  reader.consumeChar('v');
  res.positionIndex = reader.consumeChar('-') ? (d.positions.size() - reader.consumeUInt())
                                              : reader.consumeUInt() - 1;
  if (res.positionIndex < 0 || res.positionIndex >= static_cast<int>(d.positions.size())) {
    throw err::MeshErr("Position index out of bounds");
  }

  if (reader.consumeChar('/')) {
    if (*reader.getCur() != '/') {
      // Texcoord index (optionally prefixed by 'vt').
      reader.consumeChar('v');
      reader.consumeChar('t');
      res.texcoordIndex = reader.consumeChar('-') ? (d.texcoords.size() - reader.consumeUInt())
                                                  : reader.consumeUInt() - 1;
      if (res.texcoordIndex < 0 || res.texcoordIndex >= static_cast<int>(d.texcoords.size())) {
        throw err::MeshErr("Texcoord index out of bounds");
      }
    }
    if (reader.consumeChar('/')) {
      // Normal index (optionally prefixed by 'vn').
      reader.consumeChar('v');
      reader.consumeChar('n');
      res.normalIndex = reader.consumeChar('-') ? (d.normals.size() - reader.consumeUInt())
                                                : reader.consumeUInt() - 1;
      if (res.normalIndex < 0 || res.normalIndex >= static_cast<int>(d.normals.size())) {
        throw err::MeshErr("Normal index out of bounds");
      }
    }
  }
  return res;
}

/* Read obj data.
 * - vertex positions.
 * - vertex texcoords.
 * - vertex normals.
 * - faces.
 */
[[nodiscard]] auto readObjData(Reader& reader) -> ObjData {
  ObjData result = {};
  while (true) {
    switch (*reader.getCur()) {
    case ' ':
    case '\t':
    case '\n':
    case 0x0B:
    case 0x0C:
    case '\r':
      // Ascii whitespace characters: ignore.
      reader.consumeChar();
      break;
    case 'v':
      reader.consumeChar();
      switch (*reader.getCur()) {
      case ' ':
      case '\t':
        // 'v': Vertex position.
        reader.consumeWhitespace();
        result.positions.push_back(readVec3(reader));
        reader.consumeRestOfLine();
        break;
      case 't':
        // 'vt': Vertex texcoord.
        reader.consumeChar();
        reader.consumeWhitespace();
        result.texcoords.push_back(readVec2InvertY(reader));
        reader.consumeRestOfLine();
        break;
      case 'n': {
        // 'vn': Vertex normal.
        reader.consumeChar();
        reader.consumeWhitespace();
        const auto normal = readVec3(reader);
        if (normal == math::Vec3f{}) {
          // Not sure how to handle this, but there are certainly obj files that have a normal of
          // 'vn 0 0 0' defined.
          result.normals.push_back(math::dir3d::forward());
        } else {
          result.normals.push_back(normal.getNorm());
        }
        reader.consumeRestOfLine();
      } break;
      default:
        // Ignore unknown data.
        reader.consumeRestOfLine();
        break;
      }
      break;
    case 'f': {
      reader.consumeChar();
      auto face = ObjFace{static_cast<unsigned int>(result.vertices.size()), 0U, false};
      while (true) {
        reader.consumeWhitespace();
        switch (*reader.getCur()) {
        case '\r':
        case '\n':
        case '\0':
          goto FaceEnd;
        default:
          const auto v = readObjVertex(reader, result);
          result.vertices.push_back(v);
          face.useFaceNormal |= v.normalIndex == g_unusedVertexElementSentinel;
          ++face.vertexCount;
          break;
        }
      }
    FaceEnd:
      reader.consumeRestOfLine();
      result.faces.push_back(face);
    } break;
    case '\0':
      goto ObjDataEnd;
    default:
      // Ignore unknown data.
      reader.consumeRestOfLine();
      break;
    }
  }

ObjDataEnd:
  return result;
}

[[nodiscard]] auto lookupTexcoord(const ObjData& d, const ObjVertex& v) noexcept {
  if (v.texcoordIndex == g_unusedVertexElementSentinel) {
    return math::Vec2f{};
  }
  return d.texcoords[v.texcoordIndex];
}

[[nodiscard]] auto getTriSurfaceNrm(math::Vec3f posA, math::Vec3f posB, math::Vec3f posC) noexcept {
  const auto surfaceNorm = math::cross(posB - posA, posC - posA);
  if (approxZero(surfaceNorm)) {
    // Triangle with zero area has technically no normal, but does ocur in the wild.
    return math::dir3d::forward();
  }
  return surfaceNorm.getNorm();
}

} // namespace

auto loadMeshObj(log::Logger* /*unused*/, DatabaseImpl* /*unused*/, AssetId id, math::RawData raw)
    -> AssetUnique {

  // Assert that the raw buffer is null-terminated which allow us to skip bounds checks, the
  // database implementation currently guarantees that.
  assert(raw.capacity() > raw.size());
  assert(*raw.end() == '\0');

  auto vertices    = math::PodVector<Vertex>{};
  auto indices     = math::PodVector<IndexType>{};
  auto meshBuilder = MeshBuilder{&vertices, &indices};

  auto reader        = Reader{raw.begin()};
  const auto objData = readObjData(reader);
  if (objData.faces.empty()) {
    throw err::MeshErr{"No faces found in obj"};
  }

  // Triangulate all faces and push them to the meshbuilder.
  for (const auto& face : objData.faces) {
    if (face.vertexCount < 3U) {
      throw err::MeshErr{"A obj face needs to consist of atleast 3 vertices"};
    }

    math::Vec3f faceNrm;
    if (face.useFaceNormal) {
      faceNrm = getTriSurfaceNrm(
          objData.positions[objData.vertices[face.vertexIndex].positionIndex],
          objData.positions[objData.vertices[face.vertexIndex + 1U].positionIndex],
          objData.positions[objData.vertices[face.vertexIndex + 2U].positionIndex]);
    }

    // Create a triangle fan around the first vertex.
    const auto& vertA        = objData.vertices[face.vertexIndex];
    const auto vertAPos      = objData.positions[vertA.positionIndex];
    const auto vertATexcoord = lookupTexcoord(objData, vertA);
    const auto vertANorm     = face.useFaceNormal ? faceNrm : objData.normals[vertA.normalIndex];

    for (auto i = 2U; i < face.vertexCount; ++i) {
      const auto& vertB        = objData.vertices[face.vertexIndex + i - 1];
      const auto vertBPos      = objData.positions[vertB.positionIndex];
      const auto vertBTexcoord = lookupTexcoord(objData, vertB);
      const auto vertBNorm     = face.useFaceNormal ? faceNrm : objData.normals[vertB.normalIndex];

      const auto& vertC        = objData.vertices[face.vertexIndex + i];
      const auto vertCPos      = objData.positions[vertC.positionIndex];
      const auto vertCTexcoord = lookupTexcoord(objData, vertC);
      const auto vertCNorm     = face.useFaceNormal ? faceNrm : objData.normals[vertC.normalIndex];

      meshBuilder.pushVertex(Vertex{vertAPos, vertANorm, vertATexcoord});
      meshBuilder.pushVertex(Vertex{vertBPos, vertBNorm, vertBTexcoord});
      meshBuilder.pushVertex(Vertex{vertCPos, vertCNorm, vertCTexcoord});
    }
  }

  return std::make_unique<Mesh>(std::move(id), std::move(vertices), std::move(indices));
}

} // namespace tria::asset::internal

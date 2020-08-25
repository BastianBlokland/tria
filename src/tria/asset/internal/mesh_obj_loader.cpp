#include "loader.hpp"
#include "mesh_builder.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec.hpp"

namespace tria::asset::internal {

/* Wavefront Obj.
 * Format specification: http://www.martinreddy.net/gfx/3d/OBJ.spec
 * Faces are assumed to be contex and a triangulated using a simple triangle fan.
 */

namespace {

class Reader final {
public:
  Reader(const uint8_t* current) : m_cur{current} {}

  [[nodiscard]] auto getCur() -> const uint8_t*& { return m_cur; }

  /* Read a single character.
   * Last character is guaranteed to be a null-terminator, reading beyond the null-terminator is
   * undefined behaviour.
   */
  auto consumeChar() -> char { return *m_cur++; }

  /* Read a character if it matches the given character.
   */
  auto consumeChar(char c) -> bool {
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
  auto consumeWhitespace() {
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
  auto consumeRestOfLine() {
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

/* Indices for a single face vertex starting from 1.
 * Negative indices start from the end of the buffer.
 * Normal and texcoord are optional, 0 means unused.
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
auto readVec2InvertY(Reader& reader) -> math::Vec2f {
  math::Vec2f result;
  result.x() = reader.consumeFloat();
  reader.consumeWhitespace();
  result.y() = 1.f - reader.consumeFloat();
  return result;
}

/* Read x, y and z floats seperated by whitespace.
 */
auto readVec3(Reader& reader) -> math::Vec3f {
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
 */
auto readObjVertex(Reader& reader) -> ObjVertex {
  ObjVertex result = {};

  // Position index (optionally prefixed by 'v').
  reader.consumeChar('v');
  result.positionIndex = reader.consumeInt();

  if (reader.consumeChar('/')) {
    if (*reader.getCur() != '/') {
      // Texcoord index (optionally prefixed by 'vt').
      reader.consumeChar('v');
      reader.consumeChar('t');
      result.texcoordIndex = reader.consumeInt();
    }
    if (reader.consumeChar('/')) {
      // Normal index (optionally prefixed by 'vn').
      reader.consumeChar('v');
      reader.consumeChar('n');
      result.normalIndex = reader.consumeInt();
    }
  }
  return result;
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
      auto face = ObjFace{static_cast<unsigned int>(result.vertices.size()), 0U};
      while (true) {
        reader.consumeWhitespace();
        switch (*reader.getCur()) {
        case '\r':
        case '\n':
        case '\0':
          goto FaceEnd;
        default:
          result.vertices.push_back(readObjVertex(reader));
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

/* Resolve obj indices, indices start from 1 and negative indices start from the end of the data.
 * Index 0 indicates the value is not used and the default will be returned.
 */
template <typename T>
[[nodiscard]] auto resolveIndex(const math::PodVector<T>& vec, int index) {
  if (index == 0) {
    return T{};
  }
  if (index < 0) {
    if (index < -static_cast<int>(vec.size())) {
      throw err::MeshErr{"Index out of bounds"};
    }
    return vec[vec.size() + index];
  }
  if (index > static_cast<int>(vec.size())) {
    throw err::MeshErr{"Index out of bounds"};
  }
  return vec[index - 1]; // Obj indices are 1 based.
}

[[nodiscard]] auto getTriSurfaceNrm(math::Vec3f posA, math::Vec3f posB, math::Vec3f posC) {
  const auto surfaceNorm = math::cross(posB - posA, posC - posA);
  if (approxZero(surfaceNorm)) {
    // Triangle with zero area has technically no normal.
    return math::dir3d::forward();
  }
  return surfaceNorm.getNorm();
}

} // namespace

auto loadMeshObj(log::Logger* /*unused*/, DatabaseImpl* /*unused*/, AssetId id, math::RawData raw)
    -> AssetUnique {

  // Assert that the raw buffer is null-terminated which allow us to skip bounds checks, the
  // database implementation guarantees that.
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

    // Create a triangle fan for each face.
    const auto& vertA        = objData.vertices[face.vertexIndex]; // Pivot point.
    const auto vertAPos      = resolveIndex(objData.positions, vertA.positionIndex);
    const auto vertATexcoord = resolveIndex(objData.texcoords, vertA.texcoordIndex);
    auto vertANorm           = resolveIndex(objData.normals, vertA.normalIndex);

    for (auto i = 2U; i < face.vertexCount; ++i) {
      const auto& vertB        = objData.vertices[face.vertexIndex + i - 1];
      const auto vertBPos      = resolveIndex(objData.positions, vertB.positionIndex);
      const auto vertBTexcoord = resolveIndex(objData.texcoords, vertB.texcoordIndex);
      auto vertBNorm           = resolveIndex(objData.normals, vertB.normalIndex);

      const auto& vertC        = objData.vertices[face.vertexIndex + i];
      const auto vertCPos      = resolveIndex(objData.positions, vertC.positionIndex);
      const auto vertCTexcoord = resolveIndex(objData.texcoords, vertC.texcoordIndex);
      auto vertCNorm           = resolveIndex(objData.normals, vertC.normalIndex);

      // If no vertex normals are defined then use the triangle surface normal.
      if (vertANorm == math::Vec3f{} || vertBNorm == math::Vec3f{} || vertCNorm == math::Vec3f{}) {
        const auto surfNrm = getTriSurfaceNrm(vertAPos, vertBPos, vertCPos);
        vertANorm          = surfNrm;
        vertBNorm          = surfNrm;
        vertCNorm          = surfNrm;
      }

      meshBuilder.pushVertex(Vertex{vertAPos, vertANorm, vertATexcoord});
      meshBuilder.pushVertex(Vertex{vertBPos, vertBNorm, vertBTexcoord});
      meshBuilder.pushVertex(Vertex{vertCPos, vertCNorm, vertCTexcoord});
    }
  }

  return std::make_unique<Mesh>(std::move(id), std::move(vertices), std::move(indices));
}

} // namespace tria::asset::internal

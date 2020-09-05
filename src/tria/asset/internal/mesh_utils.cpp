#include "mesh_utils.hpp"
#include "tria/math/vec.hpp"

namespace tria::asset::internal {

auto computeTangents(
    math::PodVector<Vertex>& vertices, const math::PodVector<IndexType>& indices) noexcept -> void {

  /* Calculate a tangent and bitangent per triangle and accumlate the results per vertex. At the end
   * we compute a tangent per vertex by averaging the tangent and bitangents, this has the effect of
   * smoothing the tangents for vertices that are shared by multiple triangles.
   */

  auto* buffer = static_cast<math::Vec3f*>(std::calloc(vertices.size() * 2U, sizeof(math::Vec3f)));
  auto* tangents   = buffer;
  auto* bitangents = buffer + vertices.size();

  assert((indices.size() % 3U) == 0U); // Input has to be triangles.

  // Calculate per triangle tangents and bitangents and accumulate them per vertex.
  for (auto i = 0U; i != indices.size(); i += 3) {
    const auto& vA = vertices[indices[i]];
    const auto& vB = vertices[indices[i + 1U]];
    const auto& vC = vertices[indices[i + 2U]];

    const auto deltaPos1 = vB.position - vA.position;
    const auto deltaPos2 = vC.position - vA.position;
    const auto deltaTex1 = vB.texcoord - vA.texcoord;
    const auto deltaTex2 = vC.texcoord - vA.texcoord;

    const auto s = (deltaTex1.x() * deltaTex2.y() - deltaTex2.x() * deltaTex1.y());
    if (math::approxZero(s)) {
      // Not possible to calculate a tangent/bitangent here, triangle has zero texcoord area.
      continue;
    }

    const auto tan = (deltaPos1 * deltaTex2.y() - deltaPos2 * deltaTex1.y()) / s;
    tangents[indices[i]] += tan;
    tangents[indices[i + 1U]] += tan;
    tangents[indices[i + 2U]] += tan;

    const auto bitan = (deltaPos2 * deltaTex1.x() - deltaPos1 * deltaTex2.x()) / s;
    bitangents[indices[i]] += bitan;
    bitangents[indices[i + 1U]] += bitan;
    bitangents[indices[i + 2U]] += bitan;
  }

  // Write the tangents to the vertices vector.
  for (auto i = 0U; i != vertices.size(); ++i) {
    const auto& t = tangents[i];        // tangent.
    const auto& b = bitangents[i];      // bitangent.
    const auto& n = vertices[i].normal; // normal.
    if (math::approxZero(t)) {
      // Not possible to calculate a tangent, vertex is not used in any triangle with non-zero
      // positional area and texcoord area.
      vertices[i].tangent = {.1f, 0.f, 0.f, 1.f};
      continue;
    }

    // Ortho-normalize the tangent in case the texcoords are skewed.
    const auto tan = (t - project(t, n)).getNorm();

    vertices[i].tangent.x() = tan.x();
    vertices[i].tangent.y() = tan.y();
    vertices[i].tangent.z() = tan.z();

    // Calculate the 'handedness', aka if the bi-tangent needs to be flipped.
    vertices[i].tangent.w() = (dot(cross(n, t), b) < 0.f) ? 1.f : -1.f;
  }

  std::free(buffer);
}

} // namespace tria::asset::internal

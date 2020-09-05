#pragma once
#include "tria/asset/mesh.hpp"

namespace tria::asset::internal {

/* Calculate smooth tangents based on the vertex normals and texcoords.
 * Results are written to the 'tangent' property of the vertices vector, 'w' coordinate contains the
 * 'handedness' of the axis system, either '1' or '-1'.
 */
auto computeTangents(
    math::PodVector<Vertex>& vertices, const math::PodVector<IndexType>& indices) noexcept -> void;

} // namespace tria::asset::internal

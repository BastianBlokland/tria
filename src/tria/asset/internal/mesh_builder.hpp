#pragma once
#include "tria/asset/err/mesh_err.hpp"
#include "tria/asset/mesh.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace tria::asset::internal {

/* Mesh builder utility, helps with deduplicating vertices.
 * Produces a set of unique vertices and an index-buffer into those vertices.
 */
class MeshBuilder final {
public:
  MeshBuilder(math::PodVector<Vertex>* verticesOut, math::PodVector<IndexType>* indicesOut) :
      m_verticesOut{verticesOut}, m_indicesOut{indicesOut} {
    assert(m_verticesOut);
    assert(m_indicesOut);
  };
  ~MeshBuilder() = default;

  auto pushVertex(Vertex v) -> void {
    auto [itr, inserted] =
        m_uniqueVertices.insert({v, static_cast<IndexType>(m_verticesOut->size())});

    if (inserted) {
      if (m_verticesOut->size() >= std::numeric_limits<IndexType>::max()) {
        throw err::MeshErr{"Number of vertices in mesh exceeds maximum supported"};
      }
      m_verticesOut->push_back(v);
    }
    m_indicesOut->push_back(itr->second);
  }

private:
  math::PodVector<Vertex>* m_verticesOut;
  math::PodVector<IndexType>* m_indicesOut;
  std::unordered_map<Vertex, IndexType> m_uniqueVertices;
};

} // namespace tria::asset::internal

#pragma once
#include "tria/asset/err/mesh_err.hpp"
#include "tria/asset/mesh.hpp"
#include <cassert>
#include <cstdint>
#include <limits>

namespace tria::asset::internal {

namespace {

constexpr auto g_emptySlotSentinel = std::numeric_limits<IndexType>::max();

[[nodiscard]] auto hash(const Vertex& v) { return math::hash(&v, sizeof(Vertex)); }

} // namespace

/* Mesh builder utility, helps with deduplicating vertices.
 * Produces a set of unique vertices and an index-buffer into those vertices.
 */
class MeshBuilder final {
public:
  MeshBuilder(
      math::PodVector<Vertex>* verticesOut,
      math::PodVector<IndexType>* indicesOut,
      size_t verticesCount) :
      m_verticesOut{verticesOut}, m_indicesOut{indicesOut} {
    assert(m_verticesOut);
    assert(m_indicesOut);
    assert(verticesCount >= 3U);

    // Use a power of two for the table size, allows for faster modulo computation.
    m_idxLookup = {math::nextPow2(verticesCount)};
    // Initialize all slots to 'empty'.
    std::memset(m_idxLookup.data(), g_emptySlotSentinel, sizeof(IndexType) * m_idxLookup.size());
  };
  ~MeshBuilder() = default;

  auto pushVertex(const Vertex& v) -> void {
    if (m_verticesOut->size() == g_emptySlotSentinel) {
      throw err::MeshErr{"Number of vertices in mesh exceeds maximum supported"};
    }
    assert(m_indicesOut->size() < m_idxLookup.size());

    auto idx = addVertex(v);
    m_indicesOut->push_back(idx);
  }

private:
  math::PodVector<IndexType> m_idxLookup;
  math::PodVector<Vertex>* m_verticesOut;
  math::PodVector<IndexType>* m_indicesOut;

  [[nodiscard]] auto addVertex(const Vertex& vertex) noexcept -> IndexType {
    assert(m_idxLookup.size() != 0U);
    assert(math::isPow2(m_idxLookup.size()));

    /* Deduplicate using a simple open-addressing hash table.
     * https://en.wikipedia.org/wiki/Open_addressing
     */

    auto bucket = hash(vertex) & (m_idxLookup.size() - 1U);
    for (auto i = 0U; i != m_idxLookup.size(); ++i) {
      auto& slot = m_idxLookup[bucket];

      if (slot == g_emptySlotSentinel) {
        // Unique vertex, copy to vertices output and save the index in the table.
        slot = static_cast<IndexType>(m_verticesOut->size());
        m_verticesOut->push_back(vertex);
        return slot;
      }

      assert(slot < m_verticesOut->size());
      if (std::memcmp(m_verticesOut->begin() + slot, &vertex, sizeof(Vertex)) == 0) {
        // Equal to the vertex in this slot, return the index.
        return slot;
      }

      // Hash collision, jump to a new place in the table (quadratic probing).
      bucket = (bucket + i + 1U) & (m_idxLookup.size() - 1U);
    }

    assert(!"Indices lookup table is full");
    return 0U;
  }
};

} // namespace tria::asset::internal

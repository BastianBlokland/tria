#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/mesh_err.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec_io.hpp"
#include "utils.hpp"
#include <sstream>

namespace tria::asset {

// Pretty output for vertices in case of test failures.
auto operator<<(std::ostream& out, const Vertex& rhs) -> std::ostream& {
  out << "{" << rhs.position << ", " << rhs.normal << ", " << rhs.texcoord << "}";
  return out;
}

namespace tests {

// Check if vertices approximately match.
class VertexMatcher final : public Catch::MatcherBase<std::vector<Vertex>> {
public:
  explicit VertexMatcher(const std::vector<Vertex>& comparator) : m_comparator{comparator} {}

  [[nodiscard]] auto match(const std::vector<Vertex>& v) const -> bool override {
    if (m_comparator.size() != v.size()) {
      return false;
    }
    for (auto i = 0U; i != v.size(); ++i) {
      if (!approx(m_comparator[i], v[i])) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] auto describe() const -> std::string override {
    std::ostringstream ss;
    ss << "Equals: {";
    for (auto i = 0U; i != m_comparator.size(); ++i) {
      ss << m_comparator[i];
      if (i != m_comparator.size() - 1) {
        ss << ", ";
      }
    }
    ss << "}";
    return ss.str();
  }

private:
  const std::vector<Vertex>& m_comparator;
};

TEST_CASE("[asset] - Mesh Wavefront Obj", "[asset]") {

  SECTION("Vertex positions are read") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0 \n"
          "v 2.0 5.0 8.0 \n"
          "v 3.0 6.0 9.0 \n"
          "f 1 2 3 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
    });
  }

  SECTION("Vertex normals are read") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0\n"
          "v 2.0 5.0 8.0\n"
          "v 3.0 6.0 9.0\n"
          "vn 1.0 0.0 0.0\n"
          "vn 0.0 1.0 0.0\n"
          "vn 0.0 0.0 1.0\n"
          "f 1//1 2//2 3//3 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {1.f, 0.f, 0.f}, {1., 0., 0., 1.}, {}},
               {{2.0, 5.0, 8.0}, {0.f, 1.f, 0.f}, {1., 0., 0., 1.}, {}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
    });
  }

  SECTION("Face elements can be prefixed with 'v', 'vt' and 'vn'") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0\n"
          "v 2.0 5.0 8.0\n"
          "v 3.0 6.0 9.0\n"
          "vt 0.1 0.5\n"
          "vt 0.3 0.5\n"
          "vt 0.5 0.5\n"
          "vn 1.0 0.0 0.0\n"
          "vn 0.0 1.0 0.0\n"
          "vn 0.0 0.0 1.0\n"
          "f v1/vt1/vn-3 v2/vt2/vn-2 v3/vt3/vn-1 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {1.f, 0.f, 0.f}, {1., 0., 0., 1.}, {0.1, 0.5}},
               {{2.0, 5.0, 8.0}, {0.f, 1.f, 0.f}, {1., 0., 0., 1.}, {0.3, 0.5}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.5, 0.5}}}));
    });
  }

  SECTION("Texture coordinates are read") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0\n"
          "v 2.0 5.0 8.0\n"
          "v 3.0 6.0 9.0\n"
          "vt 0.1 0.5\n"
          "vt 0.3 0.5\n"
          "vt 0.5 0.5\n"
          "f 1/1 2/2 3/3 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.1, 0.5}},
               {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.3, 0.5}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.5, 0.5}}}));
    });
  }

  SECTION("Texture coordinates can be reused") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0\n"
          "v 2.0 5.0 8.0\n"
          "v 3.0 6.0 9.0\n"
          "vt 0.5 0.5\n"
          "f 1/1 2/1 3/1 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.5, 0.5}},
               {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.5, 0.5}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {0.5, 0.5}}}));
    });
  }

  SECTION("Indices are generated") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 0.0 0.0 \n"
          "v 2.0 0.0 0.0 \n"
          "v 3.0 0.0 0.0 \n"
          "f 1 2 3 \n");

      auto db      = Database{nullptr, dir};
      auto mesh    = db.get("test.obj")->downcast<Mesh>();
      auto indices = std::vector<IndexType>(mesh->getIndexBegin(), mesh->getIndexEnd());
      CHECK(indices == std::vector<IndexType>{0, 1, 2});
    });
  }

  SECTION("Faces are triangulated") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v -0.5 -0.5 0.0 \n"
          "v 0.5 -0.5 0.0 \n"
          "v -0.5 0.5 0.0 \n"
          "v 0.5 0.5 0.0 \n"
          "f 1 2 3 4 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      auto indices  = std::vector<IndexType>(mesh->getIndexBegin(), mesh->getIndexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{-0.5, -0.5, 0.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{+0.5, -0.5, 0.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{-0.5, +0.5, 0.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{+0.5, +0.5, 0.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
      CHECK(indices == std::vector<IndexType>{0, 1, 2, 0, 2, 3});
    });
  }

  SECTION("Negative indices can be used to index relatively") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 2.0 3.0 \n"
          "v 4.0 5.0 6.0 \n"
          "v 7.0 8.0 9.0 \n"
          "f -3 -2 -1 \n"
          "v 10.0 11.0 12.0 \n"
          "v 13.0 14.0 15.0 \n"
          "v 16.0 17.0 18.0 \n"
          "f -1 -2 -3 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 2.0, 3.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{4.0, 5.0, 6.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{7.0, 8.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{16.0, 17.0, 18.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{13.0, 14.0, 15.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{10.0, 11.0, 12.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
    });
  }

  SECTION("Comments are ignored") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "# Hello World\n"
          "v 1.0 4.0 7.0 \n"
          "#Another comment\n"
          "v 2.0 5.0 8.0 \n"
          "#Another comment\n"
          "#Another comment\n"
          "v 3.0 6.0 9.0 \n"
          "f 1 2 3 \n"
          "# Comment at the end");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
    });
  }

  SECTION("Whitespace is ignored") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "    v  \t  1.0  \t 4.0    7.0   \r\n"
          "\tv\t2.0\t5.0\t8.0\n"
          "\t\t v \t 3.0  6.0  9.0 \n"
          "f\t 1  \t2  \t3 \r\n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK_THAT(
          vertices,
          VertexMatcher(
              {{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}},
               {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {1., 0., 0., 1.}, {}}}));
    });
  }

  SECTION("Loading a mesh with out of bounds positive indices throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0 \n"
          "v 2.0 5.0 8.0 \n"
          "v 3.0 6.0 9.0 \n"
          "f 1 2 4 \n");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.obj"), err::MeshErr);
    });
  }

  SECTION("Loading a mesh with out of bounds negative indices throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0 \n"
          "v 2.0 5.0 8.0 \n"
          "v 3.0 6.0 9.0 \n"
          "f 1 2 -4 \n");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.obj"), err::MeshErr);
    });
  }

  SECTION("Loading a mesh from an invalid obj file throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.obj", "Hello world");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.obj"), err::MeshErr);
    });
  }

  SECTION("Loading a mesh without any faces throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v -0.5 -0.5 0.0 \n"
          "v 0.5 -0.5 0.0 \n"
          "v -0.5 0.5 0.0 \n"
          "v 0.5 0.5 0.0 \n");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.obj"), err::MeshErr);
    });
  }
}

} // namespace tests

} // namespace tria::asset

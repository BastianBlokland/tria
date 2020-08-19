#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/mesh_err.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec_io.hpp"
#include "utils.hpp"

namespace tria::asset {

// Pretty output for vertices in case of test failures.
auto operator<<(std::ostream& out, const Vertex& rhs) -> std::ostream& {
  out << "{" << rhs.position << ", " << rhs.normal << ", " << rhs.texcoord << "}";
  return out;
}

namespace tests {

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
      CHECK(
          vertices ==
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {}},
                              {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {}},
                              {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {}}});
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
      CHECK(
          vertices ==
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, {1.f, 0.f, 0.f}, {}},
                              {{2.0, 5.0, 8.0}, {0.f, 1.f, 0.f}, {}},
                              {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {}}});
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
      CHECK(
          vertices ==
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {0.1, 0.5}},
                              {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {0.3, 0.5}},
                              {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {0.5, 0.5}}});
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
      CHECK(
          vertices ==
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, {0.f, 0.f, 1.f}, {0.5, 0.5}},
                              {{2.0, 5.0, 8.0}, {0.f, 0.f, 1.f}, {0.5, 0.5}},
                              {{3.0, 6.0, 9.0}, {0.f, 0.f, 1.f}, {0.5, 0.5}}});
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

  SECTION("Faces are triangulated on read") {
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
      CHECK(
          vertices ==
          std::vector<Vertex>{{{-0.5, -0.5, 0.0}, {0.f, 0.f, 1.f}, {}},
                              {{+0.5, -0.5, 0.0}, {0.f, 0.f, 1.f}, {}},
                              {{-0.5, +0.5, 0.0}, {0.f, 0.f, 1.f}, {}},
                              {{+0.5, +0.5, 0.0}, {0.f, 0.f, 1.f}, {}}});
      CHECK(indices == std::vector<IndexType>{0, 1, 2, 0, 2, 3});
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

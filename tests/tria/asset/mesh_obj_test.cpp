#include "catch2/catch.hpp"
#include "tria/asset/database.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/mesh.hpp"
#include "tria/math/vec_io.hpp"
#include "utils.hpp"

namespace tria::asset {

// Pretty output for vertices in case of test failures.
auto operator<<(std::ostream& out, const Vertex& rhs) -> std::ostream& {
  out << "{" << rhs.position << ", " << rhs.color << "}";
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
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, math::color::white()},
                              {{2.0, 5.0, 8.0}, math::color::white()},
                              {{3.0, 6.0, 9.0}, math::color::white()}});
    });
  }

  SECTION("Vertex colors (obj extension) are read") {
    withTempDir([](const fs::path& dir) {
      writeFile(
          dir / "test.obj",
          "v 1.0 4.0 7.0 1.0 0.0 0.0 \n"
          "v 2.0 5.0 8.0 0.0 1.0 0.0 \n"
          "v 3.0 6.0 9.0 0.0 0.0 1.0 \n"
          "f 1 2 3 \n");

      auto db       = Database{nullptr, dir};
      auto mesh     = db.get("test.obj")->downcast<Mesh>();
      auto vertices = std::vector<Vertex>(mesh->getVertexBegin(), mesh->getVertexEnd());
      CHECK(
          vertices ==
          std::vector<Vertex>{{{1.0, 4.0, 7.0}, math::color::red()},
                              {{2.0, 5.0, 8.0}, math::color::lime()},
                              {{3.0, 6.0, 9.0}, math::color::blue()}});
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
          std::vector<Vertex>{{{-0.5, -0.5, 0.0}, math::color::white()},
                              {{+0.5, -0.5, 0.0}, math::color::white()},
                              {{-0.5, +0.5, 0.0}, math::color::white()},
                              {{+0.5, +0.5, 0.0}, math::color::white()}});
      CHECK(indices == std::vector<IndexType>{0, 1, 2, 0, 2, 3});
    });
  }

  SECTION("Loading a mesh from an invalid obj file throws") {
    withTempDir([](const fs::path& dir) {
      writeFile(dir / "test.obj", "Hello world");

      auto db = Database{nullptr, dir};
      CHECK_THROWS_AS(db.get("test.obj"), err::AssetLoadErr);
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
      CHECK_THROWS_AS(db.get("test.obj"), err::AssetLoadErr);
    });
  }
}

} // namespace tests

} // namespace tria::asset

#include "catch2/catch.hpp"
#include "tria/math/mat.hpp"
#include "tria/math/quat.hpp"
#include "tria/math/quat_io.hpp"
#include "tria/math/utils.hpp"
#include "tria/math/vec.hpp"

namespace tria::math::tests {

TEST_CASE("[math] - Quat", "[math]") {

  SECTION("Quaternions are fixed size") {
    auto q1 = Quatf{};
    CHECK(sizeof(q1) == sizeof(float) * 4);

    auto q2 = Quat<double>{};
    CHECK(sizeof(q2) == sizeof(double) * 4);
  }

  SECTION("Quaternions can be reassigned") {
    auto q = Quatf{};
    q      = identityQuatf();
    CHECK(q[0] == 0);
    CHECK(q[1] == 0);
    CHECK(q[2] == 0);
    CHECK(q[3] == 1);
  }

  SECTION("Quaternions can be checked for equality") {
    CHECK(identityQuatf() == identityQuatf());
    CHECK(!(Quatf{} == identityQuatf()));
  }

  SECTION("Quaternions can be checked for inequality") {
    CHECK(Quatf{} != identityQuatf());
    CHECK(!(identityQuatf() != identityQuatf()));
  }

  SECTION("Dividing a quaternion by a scalar divides each component") {
    CHECK(approx(Quatf{2, 4, 6, 2} / 2, Quatf{1, 2, 3, 1}));
  }

  SECTION("Multiplying a quaternion by a scalar multiplies each component") {
    CHECK(approx(Quatf{2, 4, 6, 2} * 2, Quatf{4, 8, 12, 4}));
  }

  SECTION("Square magnitude is sum of squared components") {
    const auto q = Quatf{1.f, 2.f, 3.f, 1.f};
    CHECK(approx(q.getSqrMag(), 15.f));
  }

  SECTION("Magnitude is the square root of the sum of squared components") {
    const auto q = Quatf{0.f, 42.f, 0.f, 0.f};
    CHECK(approx(q.getMag(), 42.0f));
  }

  SECTION("Identity quat multiplied by a identity quat returns another identity quat") {
    CHECK(approx(identityQuatf() * identityQuatf(), identityQuatf()));
  }

  SECTION("Inverse an of identity quaternion is also a identity quaternion") {
    CHECK(approx(identityQuatf().getInv(), identityQuatf()));
  }

  SECTION("Square magnitude of an identity quaternion is 1") {
    CHECK(approx(identityQuatf().getSqrMag(), 1.f));
  }

  SECTION("Multiplying an identity quaternion by a vector yields the same vector") {
    auto q = identityQuatf();
    CHECK(approx(q * dir3d::forward(), dir3d::forward()));
    CHECK(approx(q * Vec3f{.42, 13.37, -42}, Vec3f{.42, 13.37, -42}));
  }

  SECTION("Quaternions can be composed") {
    auto rot1  = angleAxisQuatf(dir3d::up(), 42.f);
    auto rot2  = angleAxisQuatf(dir3d::right(), 13.37f);
    auto comb1 = rot1 * rot2;
    auto comb2 = rot2 * rot1;
    CHECK(approx(comb1 * Vec3f{.42, 13.37, -42}, rot1 * (rot2 * Vec3f{.42, 13.37, -42}), .000001f));
    CHECK(approx(comb2 * Vec3f{.42, 13.37, -42}, rot2 * (rot1 * Vec3f{.42, 13.37, -42}), .000001f));
  }

  SECTION(
      "Multiplying a quaternion by a normalized quaternion returns in a normalized quaternion") {
    auto rot1 = angleAxisQuatf(dir3d::up(), 42.f);
    auto rot2 = angleAxisQuatf(dir3d::right(), 13.37f);
    CHECK(approx(rot1.getSqrMag(), 1.f, .000001f));
    CHECK(approx(rot2.getSqrMag(), 1.f, .000001f));
    CHECK(approx((rot1 * rot2).getSqrMag(), 1.f, .000001f));
  }

  SECTION("Inverse of a normalized quaternion is also normalized") {
    auto rot = angleAxisQuatf(Vec3f{.42, 13.37, -42}.getNorm(), 13.37f);
    CHECK(approx(rot.getSqrMag(), 1.f, .000001f));
    CHECK(approx(rot.getInv().getSqrMag(), 1.f, .000001f));
  }

  SECTION("Rotating 'left' 180 degrees over y axis results in 'right'") {
    auto rot = angleAxisQuatf(dir3d::up(), 180.f * math::degToRad<float>);
    CHECK(approx(rot * dir3d::left(), dir3d::right(), .000001f));
  }

  SECTION("Rotating 'left' 90 degrees over y axis results in 'forward'") {
    auto rot = angleAxisQuatf(dir3d::up(), 90.f * math::degToRad<float>);
    CHECK(approx(rot * dir3d::left(), dir3d::forward(), .000001f));
  }

  SECTION("Rotating 'left' by inverse of 90 degrees over y axis results in 'backward'") {
    auto rot = angleAxisQuatf(dir3d::up(), 90.f * math::degToRad<float>);
    CHECK(approx(rot.getInv() * dir3d::left(), dir3d::backward(), .000001f));
  }

  SECTION("Rotating a vector by 42 degrees results in a vector that is 42 degrees away") {
    auto rot = angleAxisQuatf(dir3d::up(), 42.f * math::degToRad<float>);
    CHECK(approx(angle(dir3d::forward(), rot * dir3d::forward()) * math::radToDeg<float>, 42.f));
    CHECK(approx(
        angle(dir3d::forward(), rot.getInv() * dir3d::forward()) * math::radToDeg<float>, 42.f));
  }

  SECTION("Quaternion angle axis over right axis provides the same results as x-rotation matrix") {
    auto rotMat  = rotXMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::right(), 42.f);

    auto vec1 = rotMat * Vec4f{.42, 13.37, -42, 0.f};
    auto vec2 = rotQuat * Vec3f{.42, 13.37, -42};
    CHECK(approx(Vec3f{vec1.x(), vec1.y(), vec1.z()}, vec2, .000001f));
  }

  SECTION("Quaternion angle axis over up axis provides the same results as y-rotation matrix") {
    auto rotMat  = rotYMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::up(), 42.f);

    auto vec1 = rotMat * Vec4f{.42, 13.37, -42, 0.f};
    auto vec2 = rotQuat * Vec3f{.42, 13.37, -42};
    CHECK(approx(Vec3f{vec1.x(), vec1.y(), vec1.z()}, vec2, .000001f));
  }

  SECTION(
      "Quaternion angle axis over forward axis provides the same results as z-rotation matrix") {
    auto rotMat  = rotZMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::forward(), 42.f);

    auto vec1 = rotMat * Vec4f{.42, 13.37, -42, 0.f};
    auto vec2 = rotQuat * Vec3f{.42, 13.37, -42};
    CHECK(approx(Vec3f{vec1.x(), vec1.y(), vec1.z()}, vec2, .000001f));
  }

  SECTION(
      "Creating rotation matrix from angle-axis over right is the same as a x-rotation matrix") {
    auto rotMat  = rotXMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::right(), 42.f);
    CHECK(approx(rotMat, rotMat4f(rotQuat), .000001f));
  }

  SECTION("Creating rotation matrix from angle-axis over up is the same as a y-rotation matrix") {
    auto rotMat  = rotYMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::up(), 42.f);
    CHECK(approx(rotMat, rotMat4f(rotQuat), .000001f));
  }

  SECTION(
      "Creating rotation matrix from angle-axis over forward is the same as a z-rotation matrix") {
    auto rotMat  = rotZMat4f(42.f);
    auto rotQuat = angleAxisQuatf(dir3d::forward(), 42.f);
    CHECK(approx(rotMat, rotMat4f(rotQuat), .000001f));
  }

  SECTION("Rotation matrix from axes is the same as from a quaternion") {
    auto rot         = angleAxisQuatf(dir3d::up(), 42.f) * angleAxisQuatf(dir3d::right(), 13.f);
    auto newRight    = rot * dir3d::right();
    auto newUp       = rot * dir3d::up();
    auto newForward  = rot * dir3d::forward();
    auto matFromAxes = rotMat4f(newRight, newUp, newForward);
    CHECK(approx(matFromAxes, rotMat4f(rot), .000001f));
  }

  SECTION("Rotate 180 degrees over y axis rotation can be retrieved from rotation matrix") {
    auto mat  = Mat3f{};
    mat[0][0] = -1.f;
    mat[1][1] = 1.f;
    mat[2][2] = -1.f;
    CHECK(approx(quatFromMat(mat), angleAxisQuatf(dir3d::up(), math::pi<float>), .000001f));
  }

  SECTION("Quaternion -> Matrix -> Quaternion produces the same value") {
    auto rotQuat1 = angleAxisQuatf(dir3d::up(), 42.f) * angleAxisQuatf(dir3d::right(), 13.f);
    auto rotQuat2 = quatFromMat(rotMat3f(rotQuat1));
    auto vec      = Vec3f{.42, 13.37, -42};
    CHECK(approx(rotQuat1 * vec, rotQuat2 * vec, .000001f));
  }

  SECTION("Quaternion created from orthogonal rotation matrix is normalized") {
    auto mat  = rotXMat3f(42.f) * rotYMat3f(-13.37f) * rotZMat3f(1.1f);
    auto quat = quatFromMat(mat);
    CHECK(approx(quat.getSqrMag(), 1.f, .000001f));
  }

  SECTION("Look rotation rotates from identity to the given axis system") {
    auto newFwd = Vec3f{.42, 13.37, -42}.getNorm();
    auto rot    = lookRotQuatf(newFwd, dir3d::up());
    CHECK(approx(rot * dir3d::forward(), newFwd, .000001f));
  }

  SECTION("Look rotation returns a normalized quaternion") {
    auto rot = lookRotQuatf(Vec3f{.42, 13.37, -42}, dir3d::down());
    CHECK(approx(rot.getSqrMag(), 1.f, .000001f));
  }

  SECTION("Look rotation is the same as building a rotation matrix from axes") {
    auto rotQuat = lookRotQuatf(dir3d::right(), dir3d::down());
    auto rotMat  = rotMat4f(dir3d::forward(), dir3d::down(), dir3d::right());
    auto vec1    = rotMat * Vec4f{.42, 13.37, -42, 0.f};
    auto vec2    = rotQuat * Vec3f{.42, 13.37, -42};
    CHECK(approx(Vec3f{vec1.x(), vec1.y(), vec1.z()}, vec2, .00001f));
  }

  SECTION("Normalizing a quaterion results in a quaternion with length 1") {
    auto q = Quatf{1337.f, 42.f, -42.f, 5.f};
    CHECK(approx(q.getNorm().getMag(), 1.f));

    q.norm();
    CHECK(approx(q.getMag(), 1.f));
  }
}

} // namespace tria::math::tests

#pragma once
#include "tria/math/mat.hpp"
#include "tria/math/quat.hpp"
#include "tria/math/vec.hpp"
#include <limits>

namespace tria::scene {

/* Camera abstraction.
 * Contains camera position, orientation and projection parameters.
 */
class Cam3d final {
public:
  Cam3d() = delete;
  Cam3d(math::Vec3f pos, math::Quatf orient, float fovDeg, float near, float far) :
      m_pos{pos},
      m_orient{orient},
      m_fov{fovDeg * math::degToRad<float>},
      m_near{near},
      m_far{far} {}

  [[nodiscard]] auto pos() const noexcept -> const math::Vec3f& { return m_pos; }
  [[nodiscard]] auto pos() noexcept -> math::Vec3f& { return m_pos; }

  [[nodiscard]] auto orient() const noexcept -> const math::Quatf& { return m_orient; }
  [[nodiscard]] auto orient() noexcept -> math::Quatf& { return m_orient; }

  [[nodiscard]] auto getRight() const noexcept { return m_orient * math::dir3d::right(); }
  [[nodiscard]] auto getUp() const noexcept { return m_orient * math::dir3d::up(); }
  [[nodiscard]] auto getFwd() const noexcept { return m_orient * math::dir3d::forward(); }

  /* Get the current field of view in degrees.
   */
  [[nodiscard]] auto getFovDeg() const noexcept { return m_fov * math::radToDeg<float>; }

  /* Update the field of view in degrees.
   */
  auto setFovDeg(float fovDeg) noexcept { m_fov = fovDeg * math::degToRad<float>; }

  /* Update the orientation to 'point' the camera at the given position.
   */
  auto lookAt(math::Vec3f point) noexcept {
    const auto toPoint = point - m_pos;
    if (toPoint.getSqrMag() > std::numeric_limits<float>::epsilon()) {
      m_orient = lookRotQuatf(toPoint, math::dir3d::up());
    }
  }

  /* View matrix, brings points from world space into view (camera) space.
   */
  [[nodiscard]] auto getViewMat() const noexcept {
    return math::rotMat4f(m_orient.getInv()) * math::transMat4f(m_pos * -1);
  }

  /* Projection matrix, brings points from view space into screen space.
   */
  [[nodiscard]] auto getProjMat(float aspect) const noexcept {
    return math::persProjVerMat4f(m_fov, aspect, m_near, m_far);
  }

  /* View-projection matrix, brings points from world space to screen space.
   */
  [[nodiscard]] auto getViewProjMat(float aspect) const noexcept {
    return getProjMat(aspect) * getViewMat();
  }

private:
  math::Vec3f m_pos;
  math::Quatf m_orient;
  float m_fov;
  float m_near;
  float m_far;
};

} // namespace tria::scene

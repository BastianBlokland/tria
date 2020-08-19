#include "tria/asset/database.hpp"
#include "tria/gfx/context.hpp"
#include "tria/math/pod_vector.hpp"
#include "tria/math/quat.hpp"
#include "tria/math/utils.hpp"
#include "tria/pal/interrupt.hpp"
#include "tria/pal/key.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include "tria/scene/cam3d.hpp"
#include <chrono>
#include <thread>
#include <vector>

using namespace std::literals;
using namespace tria;
using namespace tria::math;
using namespace std::chrono;

struct Obj final {
  const asset::Graphic* graphic;
  Vec3f pos;
  Quatf orient;
  float scale;
  float rotSpeed;

  Obj(const asset::Graphic* graphic, Vec3f pos, Quatf orient, float scale, float rotSpeed) :
      graphic{graphic}, pos{pos}, orient{orient}, scale{scale}, rotSpeed{rotSpeed} {}
};

[[nodiscard]] auto trsMat4f(Vec3f trans, Quatf rot, float scale) noexcept {
  return transMat4f(trans) * rotMat4f(rot) * scaleMat4f(scale);
}

auto runApp(pal::Platform& platform, asset::Database& db, gfx::Context& gfx) {

  auto win = platform.createWindow({1024, 1024});
  auto canvas =
      gfx.createCanvas(&win, gfx::VSyncMode::Disable, gfx::DepthMode::Enable, gfx::noneClearMask());

  auto objs = std::vector<Obj>{
      {db.get("graphics/cube.gfx")->downcast<asset::Graphic>(),
       Vec3f{0, 0, 0},
       identityQuatf(),
       1.f,
       0.f},
      {db.get("graphics/dragon.gfx")->downcast<asset::Graphic>(),
       Vec3f{3, 0, 0},
       identityQuatf(),
       4.f,
       1.f},
      {db.get("graphics/bunny.gfx")->downcast<asset::Graphic>(),
       Vec3f{-3, 0, 0},
       angleAxisQuatf(dir3d::up(), math::pi<float>),
       1.f,
       0.f},
      {db.get("graphics/head.gfx")->downcast<asset::Graphic>(),
       Vec3f{-5, 0, 0},
       identityQuatf(),
       4.f,
       1.f},
  };

  constexpr auto camVerFov         = 60.f;
  constexpr auto camZNear          = .1f;
  constexpr auto camMoveSpeed      = 10.f;
  constexpr auto camRotSensitivity = 3.f;
  auto cam = scene::Cam3d({-1, 0, -10.f}, identityQuatf(), camVerFov, camZNear);

  auto frameNum       = 0U;
  auto frameStartTime = high_resolution_clock::now();
  auto prevMousePos   = win.getMousePosNrm();
  while (!win.getIsCloseRequested() && !pal::isInterruptRequested()) {
    platform.handleEvents();

    ++frameNum;
    const auto newTime   = high_resolution_clock::now();
    const auto deltaTime = duration<float>(newTime - frameStartTime);
    frameStartTime       = newTime;

    for (auto& obj : objs) {
      obj.orient =
          (angleAxisQuatf(dir3d::up(), deltaTime.count() * obj.rotSpeed) * obj.orient).getNorm();
    }

    // Move the camera with wasd or the arrow keys.
    if (win.isKeyDown(pal::Key::W) || win.isKeyDown(pal::Key::ArrowUp)) {
      cam.pos() += cam.getFwd() * deltaTime.count() * camMoveSpeed;
    }
    if (win.isKeyDown(pal::Key::S) || win.isKeyDown(pal::Key::ArrowDown)) {
      cam.pos() -= cam.getFwd() * deltaTime.count() * camMoveSpeed;
    }
    if (win.isKeyDown(pal::Key::D) || win.isKeyDown(pal::Key::ArrowRight)) {
      cam.pos() += cam.getRight() * deltaTime.count() * camMoveSpeed;
    }
    if (win.isKeyDown(pal::Key::A) || win.isKeyDown(pal::Key::ArrowLeft)) {
      cam.pos() -= cam.getRight() * deltaTime.count() * camMoveSpeed;
    }

    auto mouseDelta = win.getMousePosNrm() - prevMousePos;
    if (win.isKeyDown(pal::Key::MouseRight) || win.isKeyDown(pal::Key::Control)) {
      // Rotate the camera based on the mouse movement.
      cam.orient() =
          (angleAxisQuatf(dir3d::up(), mouseDelta.x() * camRotSensitivity) *
           angleAxisQuatf(cam.getRight(), mouseDelta.y() * camRotSensitivity) * cam.orient())
              .getNorm();
    }
    prevMousePos = win.getMousePosNrm();

    if (frameNum % 30 == 0) {
      // Update window title every 30 frames.
      auto drawStats = canvas.getDrawStats();
      char titleBuffer[256];
      std::snprintf(
          titleBuffer,
          sizeof(titleBuffer),
          "cpu: %.2f ms, gpu: %.2f ms, tris: %llu, vertShaders: %llu, fragShaders: %llu",
          deltaTime.count() * 1'000,
          drawStats.gpuTime.count() * 1'000,
          static_cast<unsigned long long>(drawStats.inputAssemblyPrimitives),
          static_cast<unsigned long long>(drawStats.vertShaderInvocations),
          static_cast<unsigned long long>(drawStats.fragShaderInvocations));
      win.setTitle(titleBuffer);
    }

    if (canvas.drawBegin()) {
      auto vpMat = cam.getViewProjMat(win.getAspect());

      canvas.draw(db.get("graphics/sky.gfx")->downcast<asset::Graphic>(), vpMat);

      for (const auto& obj : objs) {
        canvas.draw(obj.graphic, vpMat * trsMat4f(obj.pos, obj.orient, obj.scale));
      }

      canvas.drawEnd();
    } else {
      // Unable to draw, possibly due to a minimized window.
      std::this_thread::sleep_for(100ms);
    }
  }

  return 0;
}

auto main(int /*unused*/, char* * /*unused*/) -> int {

  pal::setThreadName("tria_main_thread");
  pal::setupInterruptHandler();

  auto logger = log::Logger{log::makeConsolePrettySink(), log::makeFileJsonSink("sandbox.log")};

  int ret;
  try {
    auto platform = pal::Platform{&logger};
    auto db  = asset::Database{&logger, pal::getCurExecutablePath().parent_path() / "sandbox_data"};
    auto gfx = gfx::Context{&logger};

    LOG_I(&logger, "Sandbox startup");

    ret = runApp(platform, db, gfx);
  } catch (const std::exception& e) {
    LOG_E(&logger, "Uncaught exception", {"what", e.what()});
    ret = 1;
  } catch (...) {
    LOG_E(&logger, "Uncaught exception");
    ret = 1;
  }

  LOG_I(&logger, "Sandbox shutdown");
  return ret;
}

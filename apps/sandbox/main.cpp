#include "tria/asset/database.hpp"
#include "tria/gfx/context.hpp"
#include "tria/math/vec.hpp"
#include "tria/pal/interrupt.hpp"
#include "tria/pal/platform.hpp"
#include "tria/pal/utils.hpp"
#include <chrono>
#include <cmath>
#include <thread>

using namespace std::literals;
using namespace tria;
using namespace tria::math;
using namespace std::chrono;

struct alignas(16) ParticleData final {
  Vec2f pos;
  Vec2f velocity;
  Vec2f size;
  Vec2f screenSize;
  Color color;
  float lifetime;

  ParticleData(Vec2f pos, Vec2f velocity, Vec2f size, Color color) :
      pos{pos}, velocity{velocity}, size{size}, color{color}, lifetime{} {}
};

auto runApp(pal::Platform& platform, asset::Database& db, gfx::Context& gfx) {

  auto win    = platform.createWindow({1024, 1024});
  auto canvas = gfx.createCanvas(
      &win, gfx::VSyncMode::Enable, gfx::DepthMode::Disable, gfx::clearMask(gfx::Clear::Color));

  const auto* particleGfx = db.get("graphics/particle.gfx")->downcast<asset::Graphic>();

  constexpr auto gravity             = 600.0f;
  constexpr auto drag                = 0.002f;
  constexpr auto radius              = 7.0f;
  constexpr auto maxLifetime         = 20.0;
  constexpr auto collisionElasticity = .75f;

  auto particles = PodVector<ParticleData>{};

  auto frameNum       = 0U;
  auto frameStartTime = high_resolution_clock::now();
  while (!win.getIsCloseRequested() && !pal::isInterruptRequested()) {
    platform.handleEvents();

    ++frameNum;
    const auto newTime   = high_resolution_clock::now();
    const auto deltaTime = duration<float>(newTime - frameStartTime);
    frameStartTime       = newTime;

    // Spawn a particle at the mouse position.
    if (win.isKeyDown(pal::Key::MouseLeft)) {
      particles.push_back(ParticleData(
          win.getMousePos(), {10.0f, 10.0f}, {radius * 2.0f, radius * 2.0f}, color::get(frameNum)));
    }

    // Update all particles.
    auto windowSize = win.getSize();
    for (auto i = particles.size(); i--;) {
      auto& p = particles[i];

      // Delete when too old.
      p.lifetime += deltaTime.count();
      if (p.lifetime > maxLifetime) {
        particles.eraseIdx(i);
        continue;
      }

      // Separate from others.
      for (auto j = i + 1; j != particles.size(); ++j) {
        auto& other               = particles[j];
        const auto toOther        = other.pos - p.pos;
        const auto sqrDistToOther = toOther.getSqrMag();
        if (sqrDistToOther < (radius * 2.0f) * (radius * 2.0f)) {
          const auto distToOther = std::sqrt(sqrDistToOther);
          const auto sepDir      = sqrDistToOther == .0f ? Vec2f{0, 1} : toOther / distToOther;
          const auto overlap     = (radius * 2.0f) - distToOther;

          p.velocity = reflect(p.velocity, -sepDir) * collisionElasticity;
          p.pos -= sepDir * (overlap + .001f);

          other.velocity = reflect(other.velocity, sepDir) * collisionElasticity;
          other.pos += sepDir * (overlap + .001f);
        }
      }

      // Check the bounds of the screen.
      if (p.pos.x() < radius) {
        p.pos.x()      = radius;
        p.velocity.x() = std::abs(p.velocity.x()) * collisionElasticity;
      } else if (p.pos.y() < radius) {
        p.pos.y()      = radius;
        p.velocity.y() = std::abs(p.velocity.y()) * collisionElasticity;
      } else if (p.pos.x() > windowSize.x() - radius) {
        p.pos.x()      = windowSize.x() - radius;
        p.velocity.x() = -std::abs(p.velocity.x()) * collisionElasticity;
      } else if (p.pos.y() > windowSize.y() - radius) {
        p.pos.y()      = windowSize.y() - radius;
        p.velocity.y() = -std::abs(p.velocity.y()) * collisionElasticity;
      }

      p.velocity *= (1.0 - drag * deltaTime.count());
      p.velocity.y() += gravity * deltaTime.count();

      p.pos += p.velocity * deltaTime.count();
      p.screenSize = windowSize;
    }

    // Update window title every 30 frames.
    if (frameNum % 30 == 0) {
      auto drawStats = canvas.getDrawStats();
      char titleBuffer[256];
      std::snprintf(
          titleBuffer,
          sizeof(titleBuffer),
          "particles: %u, cpu: %.2f ms, gpu: %.2f ms, tris: %llu, vertShaders: %llu, "
          "fragShaders: %llu",
          static_cast<uint32_t>(particles.size()),
          deltaTime.count() * 1'000,
          drawStats.gpuTime.count() * 1'000,
          static_cast<unsigned long long>(drawStats.inputAssemblyPrimitives),
          static_cast<unsigned long long>(drawStats.vertShaderInvocations),
          static_cast<unsigned long long>(drawStats.fragShaderInvocations));
      win.setTitle(titleBuffer);
    }

    // Draw particles.
    if (canvas.drawBegin()) {
      canvas.draw(particleGfx, particles.begin(), particles.end());
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

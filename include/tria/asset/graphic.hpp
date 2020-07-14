#pragma once
#include "tria/asset/shader.hpp"

namespace tria::asset {

/*
 * Asset containing data needed for a drawing graphic.
 */
class Graphic final : public Asset {
public:
  Graphic(AssetId id, const Shader* vertShader, const Shader* fragShader) :
      Asset{std::move(id), getKind()}, m_vertShader{vertShader}, m_fragShader{fragShader} {}
  Graphic(const Graphic& rhs) = delete;
  Graphic(Graphic&& rhs)      = delete;
  ~Graphic() noexcept         = default;

  auto operator=(const Graphic& rhs) -> Graphic& = delete;
  auto operator=(Graphic&& rhs) -> Graphic& = delete;

  [[nodiscard]] constexpr static auto getKind() -> AssetKind { return AssetKind::Graphic; }

  [[nodiscard]] auto getVertShader() const noexcept { return m_vertShader; }
  [[nodiscard]] auto getFragShader() const noexcept { return m_fragShader; }

private:
  const Shader* m_vertShader;
  const Shader* m_fragShader;
};

} // namespace tria::asset

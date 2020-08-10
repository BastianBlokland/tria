#pragma once
#include "tria/log/api.hpp"
#include <array>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

/* Pipeline statistic types that are captured.
 */
enum StatType {
  InputAssemblyVerts      = 0U,
  InputAssemblyPrimitives = 1U,
  VertShaderInvocations   = 2U,
  FragShaderInvocations   = 3U,
};

constexpr auto g_numPipelineStatistics = 4;

/* Utility class for capturing pipline statistics.
 * At the moment only 1 capture at a time is supported, this could be extended to support multiple
 * if required.
 */
class StatRecorder final {
public:
  StatRecorder(log::Logger* logger, const Device* device);
  StatRecorder(const StatRecorder& rhs)     = delete;
  StatRecorder(StatRecorder&& rhs) noexcept = delete;
  ~StatRecorder();

  auto operator=(const StatRecorder& rhs) -> StatRecorder& = delete;
  auto operator=(StatRecorder&& rhs) noexcept -> StatRecorder& = delete;

  /* Are statistics supported on the gpu?
   */
  [[nodiscard]] auto isSupported() const noexcept -> bool { return m_vkQueryPool; }

  /* Get a result statistic for the last capture.
   * Returns 0 when statistics are not supported.
   * Note: Make sure that the gpu has finished the work before calling this.
   */
  [[nodiscard]] auto getStat(StatType stat) noexcept -> uint64_t;

  /* Reset the capture, call this before starting a new capture.
   */
  auto reset(VkCommandBuffer vkCmdBuffer) noexcept -> void;

  /* Begin capuring statistics.
   * Recorder needs to be reset before calling this again.
   * Needs to be followed by a call to 'endCapture'.
   */
  auto beginCapture(VkCommandBuffer vkCmdBuffer) noexcept -> void;

  /* Stop capturing statistics.
   */
  auto endCapture(VkCommandBuffer vkCmdBuffer) noexcept -> void;

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkQueryPool m_vkQueryPool;
  bool m_isCapturing;
  bool m_hasResults;
  std::array<uint64_t, g_numPipelineStatistics> m_results;
};

using StatRecorderUnique = std::unique_ptr<StatRecorder>;

} // namespace tria::gfx::internal

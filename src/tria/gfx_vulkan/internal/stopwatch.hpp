#pragma once
#include "tria/log/api.hpp"
#include <array>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

constexpr auto g_maxStopwatchTimestamps = 64U;

class Device;

using TimestampRecord = uint32_t;

/* Utility class for recording timestamps on the gpu.
 */
class Stopwatch final {
public:
  Stopwatch(log::Logger* logger, const Device* device);
  Stopwatch(const Stopwatch& rhs)     = delete;
  Stopwatch(Stopwatch&& rhs) noexcept = delete;
  ~Stopwatch();

  auto operator=(const Stopwatch& rhs) -> Stopwatch& = delete;
  auto operator=(Stopwatch&& rhs) noexcept -> Stopwatch& = delete;

  /* Are timestamps supported on the gpu?
   */
  [[nodiscard]] auto isSupported() const noexcept -> bool { return m_vkQueryPool; }

  /* Get the result of a previously marked timestamp (in nanoseconds).
   * Returns 0 when timestamps are not supported.
   * Note: Make sure that the gpu has finished the work before calling this.
   */
  [[nodiscard]] auto getTimestamp(TimestampRecord id) -> double;

  /* Reset all timestamps, call this before marking new timestamps.
   */
  auto reset(VkCommandBuffer vkCmdBuffer) noexcept -> void;

  /* Mark a timestamp to be recorded.
   * Time will be taken after all previously recorded commands have finished executing.
   * Returns a token that can be used to retreive the timestamp when rendering has finished.
   */
  [[nodiscard]] auto markTimestamp(VkCommandBuffer vkCmdBuffer) noexcept -> TimestampRecord;

private:
  log::Logger* m_logger;
  const Device* m_device;
  VkQueryPool m_vkQueryPool;
  uint32_t m_counter;
  bool m_hasResults;
  std::array<uint64_t, g_maxStopwatchTimestamps> m_results;
};

using StopwatchUnique = std::unique_ptr<Stopwatch>;

} // namespace tria::gfx::internal

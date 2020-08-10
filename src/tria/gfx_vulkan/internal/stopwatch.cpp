#include "stopwatch.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkQueryPool(VkDevice vkDevice, uint32_t maxTimestamps) -> VkQueryPool {
  VkQueryPoolCreateInfo createInfo = {};
  createInfo.sType                 = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  createInfo.queryType             = VK_QUERY_TYPE_TIMESTAMP;
  createInfo.queryCount            = maxTimestamps;

  VkQueryPool result;
  checkVkResult(vkCreateQueryPool(vkDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

Stopwatch::Stopwatch(log::Logger* logger, const Device* device) :
    m_logger{logger}, m_device{device}, m_counter{0}, m_hasResults{true} {
  assert(m_device);

  if (m_device->getLimits().timestampComputeAndGraphics) {
    m_vkQueryPool = createVkQueryPool(m_device->getVkDevice(), g_maxStopwatchTimestamps);
    DBG_QUERYPOOL_NAME(device, m_vkQueryPool, "stopwatch");
  } else {
    m_vkQueryPool = nullptr;
    LOG_W(m_logger, "Timestamps are not supported on the current device");
  }
}

Stopwatch::~Stopwatch() {
  if (m_vkQueryPool) {
    vkDestroyQueryPool(m_device->getVkDevice(), m_vkQueryPool, nullptr);
  }
}

auto Stopwatch::getTimestamp(uint32_t id) -> double {
  assert(id < m_counter);

  if (!m_vkQueryPool) {
    // Just return empty if the device does not support querying timestamps. Callers can check
    // 'isSupported' to know if valid timestamps will be returned.
    return 0.0f;
  }

  if (!m_hasResults) {
    checkVkResult(vkGetQueryPoolResults(
        m_device->getVkDevice(),
        m_vkQueryPool,
        0U,
        m_counter,
        sizeof(m_results),
        m_results.data(),
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT));
    m_hasResults = true;
  }
  return static_cast<double>(m_results[id]) * m_device->getLimits().timestampPeriod;
}

auto Stopwatch::reset(VkCommandBuffer cmdBuffer) noexcept -> void {
  if (m_vkQueryPool) {
    vkCmdResetQueryPool(cmdBuffer, m_vkQueryPool, 0U, g_maxStopwatchTimestamps);
  }
  m_counter    = 0U;
  m_hasResults = false;
}

auto Stopwatch::markTimestamp(VkCommandBuffer cmdBuffer) noexcept -> uint32_t {
  assert(!m_hasResults); // Needs to be reset before marking new timestamps.
  if (m_vkQueryPool) {
    // Note: Record the timestamp after all commands have completely finished executing (bottom of
    // pipe).
    vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_vkQueryPool, m_counter);
  }
  return m_counter++;
}

} // namespace tria::gfx::internal

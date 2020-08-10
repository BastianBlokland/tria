#include "stat_recorder.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "tria/math/utils.hpp"
#include "utils.hpp"
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkQueryPool(VkDevice vkDevice) -> VkQueryPool {
  VkQueryPoolCreateInfo createInfo = {};
  createInfo.sType                 = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  createInfo.queryType             = VK_QUERY_TYPE_PIPELINE_STATISTICS;
  createInfo.queryCount            = g_numPipelineStatistics;
  createInfo.pipelineStatistics =
      (VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
       VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
       VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
       VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT);

  // 1 query per enabled stat.
  assert(tria::math::popCount(createInfo.pipelineStatistics) == g_numPipelineStatistics);

  VkQueryPool result;
  checkVkResult(vkCreateQueryPool(vkDevice, &createInfo, nullptr, &result));
  return result;
}

} // namespace

StatRecorder::StatRecorder(log::Logger* logger, const Device* device) :
    m_logger{logger}, m_device{device}, m_isCapturing{false} {
  assert(m_device);

  if (m_device->getFeatures().pipelineStatisticsQuery) {
    m_vkQueryPool = createVkQueryPool(m_device->getVkDevice());
    DBG_QUERYPOOL_NAME(device, m_vkQueryPool, "statrecorder");
  } else {
    m_vkQueryPool = nullptr;
    LOG_W(m_logger, "Pipeline statistics are not supported on the current device");
  }
}

StatRecorder::~StatRecorder() {
  if (m_vkQueryPool) {
    vkDestroyQueryPool(m_device->getVkDevice(), m_vkQueryPool, nullptr);
  }
}

auto StatRecorder::getStat(StatType statType) noexcept -> uint64_t {
  assert(static_cast<uint32_t>(statType) < g_numPipelineStatistics);

  if (!m_vkQueryPool) {
    // Just return zero if the device does not support statistics. Callers can check 'isSupported'
    // to know if valid statistics will be returned.
    return 0;
  }

  if (!m_hasResults) {
    checkVkResult(vkGetQueryPoolResults(
        m_device->getVkDevice(),
        m_vkQueryPool,
        0U,
        1U,
        sizeof(m_results),
        m_results.data(),
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT));
    m_hasResults = true;
  }
  return m_results[static_cast<size_t>(statType)];
}

auto StatRecorder::reset(VkCommandBuffer vkCmdBuffer) noexcept -> void {
  if (m_vkQueryPool) {
    vkCmdResetQueryPool(vkCmdBuffer, m_vkQueryPool, 0U, g_numPipelineStatistics);
  }
  m_hasResults = false;
}

auto StatRecorder::beginCapture(VkCommandBuffer vkCmdBuffer) noexcept -> void {
  assert(!m_hasResults); // Needs to be reset before starting a new capture.
  assert(!m_isCapturing);
  if (m_vkQueryPool) {
    vkCmdBeginQuery(vkCmdBuffer, m_vkQueryPool, 0U, 0U);
  }
  m_isCapturing = true;
}

auto StatRecorder::endCapture(VkCommandBuffer vkCmdBuffer) noexcept -> void {
  assert(m_isCapturing);
  if (m_vkQueryPool) {
    vkCmdEndQuery(vkCmdBuffer, m_vkQueryPool, 0U);
  }
  m_isCapturing = false;
}

} // namespace tria::gfx::internal

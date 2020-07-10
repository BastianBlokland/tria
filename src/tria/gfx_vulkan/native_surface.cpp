#include "native_surface.hpp"
#include "internal/utils.hpp"
#include "native_context.hpp"
#include "tria/pal/native.hpp"
#include "tria/pal/utils.hpp"
#include <array>
#include <cassert>

namespace tria::gfx {

using namespace internal;

namespace {

[[nodiscard]] auto makeVkSurfaceKhr(const NativeContext* context, const pal::Window* window)
    -> VkSurfaceKHR {
  VkSurfaceKHR result;
#if defined(TRIA_LINUX_XCB)
  VkXcbSurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.connection                = pal::getLinuxXcbConnection(*window);
  createInfo.window                    = pal::getLinuxXcbWindow(*window);
  checkVkResult(vkCreateXcbSurfaceKHR(context->getVkInstance(), &createInfo, nullptr, &result));
#elif defined(TRIA_WIN32)
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance                   = pal::getWin32HInstance(*window);
  createInfo.hwnd                        = pal::getWin32HWnd(*window);
  checkVkResult(vkCreateWin32SurfaceKHR(context->getVkInstance(), &createInfo, nullptr, &result));
#else
  static_assert("false", "Unsupported platform");
#endif
  return result;
}

} // namespace

NativeSurface::NativeSurface(
    log::Logger* logger, const NativeContext* context, const pal::Window* window) :
    m_logger{logger}, m_context{context}, m_window{window} {
  assert(m_context);
  assert(m_window);

  m_vkSurface = makeVkSurfaceKhr(m_context, m_window);
  try {
    m_device = getDevice(m_logger, context->getVkInstance(), m_vkSurface);
    if (!m_device) {
      throw err::DriverErr{"No device found with vulkan support"};
    }
  } catch (...) {
    // Cleanup up already created resources.
    vkDestroySurfaceKHR(m_context->getVkInstance(), m_vkSurface, nullptr);
    throw;
  }

  m_device->initSwapchain(window->getWidth(), window->getHeight());

  m_triangleVertShader = loadShaderAsset(
      m_device.get(), pal::getCurExecutablePath().parent_path() / "data/shaders/triangle.vert.spv");
  m_triangleFragShader = loadShaderAsset(
      m_device.get(), pal::getCurExecutablePath().parent_path() / "data/shaders/triangle.frag.spv");

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = m_triangleVertShader->getModule();
  vertShaderStageInfo.pName  = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = m_triangleFragShader->getModule();
  fragShaderStageInfo.pName  = "main";

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
      vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x          = 0.0f;
  viewport.y          = 0.0f;
  viewport.width      = static_cast<float>(m_device->getSwapWidth());
  viewport.height     = static_cast<float>(m_device->getSwapHeight());
  viewport.minDepth   = 0.0f;
  viewport.maxDepth   = 1.0f;

  VkRect2D scissor = {};
  scissor.offset   = {0, 0};
  scissor.extent   = m_device->getSwapExtent();

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth               = 1.0f;
  rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable         = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable  = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable   = VK_FALSE;
  colorBlending.logicOp         = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments    = &colorBlendAttachment;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  checkVkResult(vkCreatePipelineLayout(
      m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount                   = shaderStages.size();
  pipelineInfo.pStages                      = shaderStages.data();
  pipelineInfo.pVertexInputState            = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState          = &inputAssembly;
  pipelineInfo.pViewportState               = &viewportState;
  pipelineInfo.pRasterizationState          = &rasterizer;
  pipelineInfo.pMultisampleState            = &multisampling;
  pipelineInfo.pColorBlendState             = &colorBlending;
  pipelineInfo.layout                       = m_pipelineLayout;
  pipelineInfo.renderPass                   = m_device->getVkRenderpass();
  pipelineInfo.subpass                      = 0;

  checkVkResult(vkCreateGraphicsPipelines(
      m_device->getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));
}

NativeSurface::~NativeSurface() {
  vkDestroyPipeline(m_device->getVkDevice(), m_graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(m_device->getVkDevice(), m_pipelineLayout, nullptr);

  m_triangleVertShader = nullptr;
  m_triangleFragShader = nullptr;

  m_device = nullptr;
  vkDestroySurfaceKHR(m_context->getVkInstance(), m_vkSurface, nullptr);
}

} // namespace tria::gfx

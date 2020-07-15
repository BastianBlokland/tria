#include "graphic.hpp"
#include "renderer.hpp"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include <array>
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createShaderModule(VkDevice vkDevice, const asset::Shader& asset) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize                 = asset.getSize();
  createInfo.pCode                    = reinterpret_cast<const uint32_t*>(asset.getData());
  VkShaderModule result;
  checkVkResult(vkCreateShaderModule(vkDevice, &createInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createPipelineLayout(VkDevice vkDevice) {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkPipelineLayout result;
  checkVkResult(vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createPipeline(
    VkDevice vkDevice,
    VkRenderPass vkRenderPass,
    VkPipelineLayout layout,
    VkShaderModule vertShader,
    VkShaderModule fragShader) {

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShader;
  vertShaderStageInfo.pName  = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShader;
  fragShaderStageInfo.pName  = "main";

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
      vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Note: viewport and scissor are set dynamically at draw time.
  VkViewport viewport = {};
  VkRect2D scissor    = {};

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth   = 1.0f;
  rasterizer.cullMode    = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace   = VK_FRONT_FACE_CLOCKWISE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable  = false;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = false;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable   = false;
  colorBlending.logicOp         = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments    = &colorBlendAttachment;

  std::array<VkDynamicState, 2> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
  dynamicStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = dynamicStates.size();
  dynamicStateInfo.pDynamicStates    = dynamicStates.data();

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount                   = shaderStages.size();
  pipelineInfo.pStages                      = shaderStages.data();
  pipelineInfo.pVertexInputState            = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState          = &inputAssembly;
  pipelineInfo.pViewportState               = &viewportState;
  pipelineInfo.pRasterizationState          = &rasterizer;
  pipelineInfo.pMultisampleState            = &multisampling;
  pipelineInfo.pDepthStencilState           = nullptr;
  pipelineInfo.pColorBlendState             = &colorBlending;
  pipelineInfo.pDynamicState                = &dynamicStateInfo;
  pipelineInfo.layout                       = layout;
  pipelineInfo.renderPass                   = vkRenderPass;
  pipelineInfo.subpass                      = 0;

  VkPipeline result;
  checkVkResult(vkCreateGraphicsPipelines(vkDevice, nullptr, 1, &pipelineInfo, nullptr, &result));
  return result;
}

} // namespace

Graphic::Graphic(
    log::Logger* logger,
    const Device* device,
    VkRenderPass vkRenderPass,
    const asset::Graphic* asset) :
    m_logger{logger}, m_device{device} {
  assert(device);

  // Create shaders.
  m_vertShader = createShaderModule(m_device->getVkDevice(), *asset->getVertShader());
  m_fragShader = createShaderModule(m_device->getVkDevice(), *asset->getFragShader());

  // Create pipeline layout.
  m_pipelineLayout = createPipelineLayout(m_device->getVkDevice());

  // Create pipeline.
  m_pipeline = createPipeline(
      m_device->getVkDevice(), vkRenderPass, m_pipelineLayout, m_vertShader, m_fragShader);

  LOG_D(m_logger, "Graphic pipline created", {"asset", asset->getId()});
}

Graphic::~Graphic() {
  vkDestroyPipeline(m_device->getVkDevice(), m_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device->getVkDevice(), m_pipelineLayout, nullptr);

  vkDestroyShaderModule(m_device->getVkDevice(), m_vertShader, nullptr);
  vkDestroyShaderModule(m_device->getVkDevice(), m_fragShader, nullptr);
}

} // namespace tria::gfx::internal

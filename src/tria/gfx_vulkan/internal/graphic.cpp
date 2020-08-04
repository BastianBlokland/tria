#include "graphic.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>

namespace tria::gfx::internal {

namespace {

template <uint32_t DescriptorSetCount>
[[nodiscard]] auto createPipelineLayout(
    VkDevice vkDevice, const std::array<VkDescriptorSetLayout, DescriptorSetCount> descLayouts) {

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount             = DescriptorSetCount;
  pipelineLayoutInfo.pSetLayouts                = descLayouts.data();

  VkPipelineLayout result;
  checkVkResult(vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createPipeline(
    VkDevice vkDevice,
    VkRenderPass vkRenderPass,
    VkPipelineLayout layout,
    const Shader* vertShader,
    const Shader* fragShader,
    const Mesh* mesh,
    asset::Graphic::BlendMode blendMode) {

  assert(vertShader);
  assert(fragShader);
  assert(mesh);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShader->getVkModule();
  vertShaderStageInfo.pName  = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShader->getVkModule();
  fragShaderStageInfo.pName  = "main";

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
      vertShaderStageInfo,
      fragShaderStageInfo,
  };

  auto vertexBindingDescriptions   = mesh->getVkVertexBindingDescriptions();
  auto vertexAttributeDescriptions = mesh->getVkVertexAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount   = vertexBindingDescriptions.size();
  vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
  vertexInputInfo.pVertexBindingDescriptions      = vertexBindingDescriptions.data();
  vertexInputInfo.pVertexAttributeDescriptions    = vertexAttributeDescriptions.data();

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
  switch (blendMode) {
  case asset::Graphic::BlendMode::Alpha:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::Graphic::BlendMode::Additive:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::Graphic::BlendMode::AlphaAdditive:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::Graphic::BlendMode::None:
  default:
    colorBlendAttachment.blendEnable = false;
    break;
  }

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable   = false;
  colorBlending.logicOp         = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments    = &colorBlendAttachment;

  std::array<VkDynamicState, 2> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
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
    Device* device,
    const asset::Graphic* asset,
    AssetResource<Shader>* shaders,
    AssetResource<Mesh>* meshes,
    AssetResource<Texture>* textures) :
    m_logger{logger}, m_device{device}, m_asset{asset}, m_vkPipeline{nullptr} {
  assert(m_device);
  assert(m_asset);

  m_vertShader = shaders->get(m_asset->getVertShader());
  m_fragShader = shaders->get(m_asset->getFragShader());
  m_mesh       = meshes->get(m_asset->getMesh());

  // Create a descriptor for the per graphic resources.
  m_descSet = device->getDescManager().allocate(
      DescriptorInfo{static_cast<uint32_t>(asset->getSamplerCount()), 0U, 0U});

  // Create the texture resources and bind them to our descriptor.
  m_textures.reserve(m_asset->getSamplerCount());
  auto dstBinding = 0U;
  for (auto itr = m_asset->getSamplerBegin(); itr != m_asset->getSamplerEnd(); ++itr) {
    // Create a gpu resource for the texture.
    const auto* tex = textures->get(itr->getTexture());

    const auto filterMode = static_cast<SamplerFilterMode>(itr->getFilterMode());
    auto sampler          = Sampler{device, filterMode, tex->getImage().getMipLevels()};
    DBG_SAMPLER_NAME(m_device, sampler.getVkSampler(), m_asset->getId());

    m_descSet.bindImage(dstBinding++, tex->getImage(), sampler);
    m_textures.emplace_back(tex, std::move(sampler));
  }
}

Graphic::~Graphic() {
  if (m_vkPipeline) {
    vkDestroyPipelineLayout(m_device->getVkDevice(), m_vkPipelineLayout, nullptr);
    vkDestroyPipeline(m_device->getVkDevice(), m_vkPipeline, nullptr);
  }
}

auto Graphic::prepareResources(
    Transferer* transferer, UniformContainer* uni, VkRenderPass vkRenderPass) const -> void {

  m_mesh->prepareResources(transferer);
  for (const auto& texData : m_textures) {
    texData.texture->prepareResources(transferer);
  }

  if (!m_vkPipeline) {
    // Bind to our own resources like textures (if any) and any per-draw uniform data.
    if (m_descSet.hasBindingPoints()) {
      m_vkPipelineLayout = createPipelineLayout<2>(
          m_device->getVkDevice(), {m_descSet.getVkLayout(), uni->getVkDescLayout()});
    } else {
      m_vkPipelineLayout =
          createPipelineLayout<1>(m_device->getVkDevice(), {uni->getVkDescLayout()});
    }
    m_vkPipeline = createPipeline(
        m_device->getVkDevice(),
        vkRenderPass,
        m_vkPipelineLayout,
        m_vertShader,
        m_fragShader,
        m_mesh,
        m_asset->getBlendMode());

    DBG_PIPELINELAYOUT_NAME(m_device, m_vkPipelineLayout, m_asset->getId());
    DBG_PIPELINE_NAME(m_device, m_vkPipeline, m_asset->getId());

    LOG_D(
        m_logger,
        "Vulkan pipline created",
        {"asset", m_asset->getId()},
        {"texCount", m_textures.size()});
  }
}

} // namespace tria::gfx::internal

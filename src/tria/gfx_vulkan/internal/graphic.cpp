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

[[nodiscard]] auto createPipelineLayout(
    VkDevice vkDevice,
    VkDescriptorSetLayout graphicDescriptor,
    VkDescriptorSetLayout instanceDescriptor) {

  std::array<VkDescriptorSetLayout, 2U> descriptorLayouts = {
      graphicDescriptor,
      instanceDescriptor,
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount             = descriptorLayouts.size();
  pipelineLayoutInfo.pSetLayouts                = descriptorLayouts.data();

  VkPipelineLayout result;
  checkVkResult(vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createPipeline(
    VkDevice vkDevice,
    VkRenderPass vkRenderPass,
    VkPipelineLayout layout,
    const std::vector<const Shader*>& shaders,
    asset::BlendMode blendMode,
    asset::DepthTestMode depthTestMode) {

  auto shaderStages = std::vector<VkPipelineShaderStageCreateInfo>();
  shaderStages.reserve(shaders.size());
  for (const auto* shader : shaders) {
    VkPipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage                           = shader->getVkStage();
    stageInfo.module                          = shader->getVkModule();
    stageInfo.pName                           = shader->getEntryPointName().data();
    shaderStages.push_back(stageInfo);
  }

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

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  switch (depthTestMode) {
  case asset::DepthTestMode::Less:
    depthStencil.depthTestEnable = true;
    depthStencil.depthCompareOp  = VK_COMPARE_OP_LESS;
    break;
  case asset::DepthTestMode::Always:
    depthStencil.depthTestEnable = true;
    depthStencil.depthCompareOp  = VK_COMPARE_OP_ALWAYS;
    break;
  case asset::DepthTestMode::None:
  default:
    depthStencil.depthTestEnable = false;
    break;
  }
  depthStencil.depthWriteEnable      = true;
  depthStencil.depthBoundsTestEnable = false;
  depthStencil.stencilTestEnable     = false;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  switch (blendMode) {
  case asset::BlendMode::Alpha:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::BlendMode::Additive:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::BlendMode::AlphaAdditive:
    colorBlendAttachment.blendEnable         = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    break;
  case asset::BlendMode::None:
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
  pipelineInfo.pDepthStencilState           = &depthStencil;
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

  for (auto* shdItr = asset->getShaderBegin(); shdItr != asset->getShaderEnd(); ++shdItr) {
    m_shaders.push_back(shaders->get(*shdItr));
  }
  m_mesh = meshes->get(m_asset->getMesh());

  // Create a descriptor for the per graphic resources.
  m_descSet = device->getDescManager().allocate(
      DescriptorInfo{1U, 0U, static_cast<uint32_t>(asset->getSamplerCount())});

  auto dstBinding = 0U;
  m_descSet.attachStorageBuffer(dstBinding++, m_mesh->getVertexBuffer());

  // Create the texture resources and bind them to our descriptor.
  m_textures.reserve(m_asset->getSamplerCount());
  for (auto itr = m_asset->getSamplerBegin(); itr != m_asset->getSamplerEnd(); ++itr) {
    // Create a gpu resource for the texture.
    const auto* tex = textures->get(itr->getTexture());

    const auto filterMode = static_cast<SamplerFilterMode>(itr->getFilterMode());
    const auto anisoMode  = static_cast<SamplerAnisotropyMode>(itr->getAnisoMode());
    auto sampler          = Sampler{device, filterMode, anisoMode, tex->getImage().getMipLevels()};
    DBG_SAMPLER_NAME(m_device, sampler.getVkSampler(), m_asset->getId());

    m_descSet.attachImage(dstBinding++, tex->getImage(), sampler);
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
    // Bind to our own resources like vertex data and any per-draw uniform data.
    m_vkPipelineLayout = createPipelineLayout(
        m_device->getVkDevice(), m_descSet.getVkLayout(), uni->getVkDescLayout());

    m_vkPipeline = createPipeline(
        m_device->getVkDevice(),
        vkRenderPass,
        m_vkPipelineLayout,
        m_shaders,
        m_asset->getBlendMode(),
        m_asset->getDepthTestMode());

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

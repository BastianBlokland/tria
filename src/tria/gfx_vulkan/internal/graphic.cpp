#include "graphic.hpp"
#include "debug_utils.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "tria/gfx/err/graphic_err.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>

namespace tria::gfx::internal {

namespace {

/* Graphic pipelines get three descriptors bound to them:
 * - 'globalDescriptor' (shared between pipelines).
 * - 'graphicDescriptor' (per graphic data like mesh and textures).
 * - 'instanceDescriptor' (per instance data like a transformation matrix).
 */
[[nodiscard]] auto createPipelineLayout(
    const Device* device,
    VkDescriptorSetLayout globalDescriptor,
    VkDescriptorSetLayout graphicDescriptor,
    VkDescriptorSetLayout instanceDescriptor) {

  std::array<VkDescriptorSetLayout, 3U> descriptorLayouts = {
      globalDescriptor,
      graphicDescriptor,
      instanceDescriptor,
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount             = descriptorLayouts.size();
  pipelineLayoutInfo.pSetLayouts                = descriptorLayouts.data();

  VkPipelineLayout result;
  checkVkResult(
      vkCreatePipelineLayout(device->getVkDevice(), &pipelineLayoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createPipeline(
    const Device* device,
    VkRenderPass vkRenderPass,
    VkSampleCount sampleCount,
    VkPipelineLayout layout,
    const std::vector<const Shader*>& shaders,
    asset::VertexTopology topology,
    asset::RasterizerMode rasterizerMode,
    float lineWidth,
    asset::BlendMode blendMode,
    asset::DepthTestMode depthTestMode,
    asset::CullMode cullMode) {

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

  // We don't use the input assembler to feed vertex data to the shaders, instead we bind a storage
  // buffer containing vertex data.
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  switch (topology) {
  case asset::VertexTopology::Triangles:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    break;
  case asset::VertexTopology::Lines:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    break;
  case asset::VertexTopology::LineStrip:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    break;
  }

  // Note: viewport and scissor are set dynamically at draw time.
  VkViewport viewport                             = {};
  VkRect2D scissor                                = {};
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1U;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1U;
  viewportState.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  if (device->getFeatures().fillModeNonSolid) {
    switch (rasterizerMode) {
    case asset::RasterizerMode::Fill:
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      break;
    case asset::RasterizerMode::Lines:
      rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
      break;
    case asset::RasterizerMode::Points:
      rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
      break;
    }
  }
  rasterizer.lineWidth = device->getFeatures().wideLines
      ? std::clamp(
            lineWidth, device->getLimits().lineWidthRange[0], device->getLimits().lineWidthRange[1])
      : 1.f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  switch (cullMode) {
  case asset::CullMode::None:
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    break;
  case asset::CullMode::Back:
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    break;
  case asset::CullMode::Front:
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    break;
  }
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable  = false;
  multisampling.rasterizationSamples = sampleCount;

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  switch (depthTestMode) {
  case asset::DepthTestMode::Less:
    depthStencil.depthTestEnable = true;
    // Use the 'greater' compare op, because we are using a reversed-z depthbuffer.
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
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
  colorBlendAttachment.colorWriteMask =
      (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
       VK_COLOR_COMPONENT_A_BIT);
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

  // Viewport and scissor are bound dynamically so avoid having to recreate all pipelines when
  // resizing the render target.
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
  checkVkResult(vkCreateGraphicsPipelines(
      device->getVkDevice(), device->getVkPipelineCache(), 1, &pipelineInfo, nullptr, &result));
  return result;
}

/* Map a ShaderSourceKind to a DescriptorBindingKind.
 */
[[nodiscard]] auto getDescBindingKind(const asset::ShaderResourceKind shaderResourceKind) noexcept {
  switch (shaderResourceKind) {
  case asset::ShaderResourceKind::Texture:
    return DescriptorBindingKind::CombinedImageSampler;
  case asset::ShaderResourceKind::UniformBuffer:
    // TODO(bastian): This makes the assumption that all uniform-buffers will be bound as dynamic
    // uniform buffers.
    return DescriptorBindingKind::UniformBufferDynamic;
  case asset::ShaderResourceKind::StorageBuffer:
    return DescriptorBindingKind::StorageBuffer;
  }
  assert(!"Unsupported ShaderResourceKind");
  return DescriptorBindingKind::UniformBuffer;
}

/* Gather all bindings that the shader exposes in the given set.
 */
[[nodiscard]] auto getDescSetBindings(uint32_t setId, const asset::Shader* shader) {
  auto bindings = DescriptorBindings{};
  for (auto* resItr = shader->getResourceBegin(); resItr != shader->getResourceEnd(); ++resItr) {
    const auto& resource = *resItr;
    if (resource.getSet() == setId) {
      bindings.emplace(resource.getBinding(), getDescBindingKind(resource.getKind()));
    }
  }
  return bindings;
}

/* Gather all bindings that the shaders expose in the given set.
 */
[[nodiscard]] auto getDescSetBindings(
    uint32_t setId,
    const asset::Shader* const* shaderBegin,
    const asset::Shader* const* shaderEnd) {
  auto bindings = DescriptorBindings{};
  for (auto* shaderItr = shaderBegin; shaderItr != shaderEnd; ++shaderItr) {
    // TODO(bastian): We should probably throw if multiple shaders define the same binding as a
    // different kind? Currently the earlier shader would win.
    bindings.merge(getDescSetBindings(setId, *shaderItr));
  }
  return bindings;
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

  // Create shader resources.
  for (auto* shdItr = asset->getShaderBegin(); shdItr != asset->getShaderEnd(); ++shdItr) {
    m_shaders.push_back(shaders->get(*shdItr));
  }

  // Create a descriptor for the 'per graphic' resources.
  auto graphicBindings = getDescSetBindings(
      g_shaderResourceGraphicSetId, asset->getShaderBegin(), asset->getShaderEnd());
  m_descSet = device->getDescManager().allocate(graphicBindings);

  // Create mesh resource (if any).
  m_mesh = m_asset->getMesh() ? meshes->get(m_asset->getMesh()) : nullptr;

  // Bind vertex data.
  if (!graphicBindings.empty() &&
      graphicBindings.begin()->second == DescriptorBindingKind::StorageBuffer) {
    const auto binding = graphicBindings.begin()->first;
    if (!m_mesh) {
      throw err::GraphicErr{asset->getId(),
                            "Shader takes a mesh input but the graphic doesn't have a mesh"};
    }
    m_descSet.attachBuffer(binding, m_mesh->getVertexBuffer(), m_mesh->getVertexBuffer().getSize());
  }

  // Create the texture resources.
  m_textures.reserve(m_asset->getSamplerCount());
  for (auto itr = m_asset->getSamplerBegin(); itr != m_asset->getSamplerEnd(); ++itr) {
    const auto* tex = textures->get(itr->getTexture());

    const auto wrapMode   = static_cast<SamplerWrapMode>(itr->getWrapMode());
    const auto filterMode = static_cast<SamplerFilterMode>(itr->getFilterMode());
    const auto anisoMode  = static_cast<SamplerAnisotropyMode>(itr->getAnisoMode());

    auto sampler = Sampler{device, wrapMode, filterMode, anisoMode, tex->getImage().getMipLevels()};
    DBG_SAMPLER_NAME(m_device, sampler.getVkSampler(), m_asset->getId());

    m_textures.push_back({tex, std::move(sampler)});
  }

  // Bind the texture resources to our descriptor.
  auto textureIdx = 0U;
  for (const auto& binding : graphicBindings) {
    if (binding.second == DescriptorBindingKind::CombinedImageSampler) {
      if (m_textures.size() == textureIdx) {
        throw err::GraphicErr{asset->getId(),
                              "Graphic does not have enough samplers to satisfy shader inputs"};
      }
      const auto& tex = m_textures[textureIdx++];
      m_descSet.attachImage(binding.first, tex.texture->getImage(), tex.sampler);
    }
  }

  // Check if the shaders uses global data.
  auto globalBindings = getDescSetBindings(
      g_shaderResourceGlobalSetId, asset->getShaderBegin(), asset->getShaderEnd());
  m_usesGlobalData = !globalBindings.empty();
  if (m_usesGlobalData) {
    const auto& binding = *globalBindings.begin();
    if (binding.first != 0U || binding.second != DescriptorBindingKind::UniformBufferDynamic) {
      throw err::GraphicErr{m_asset->getId(), "Invalid global binding"};
    }
  }

  // Check if the shaders uses instance data.
  auto instanceBindings = getDescSetBindings(
      g_shaderResourceInstanceSetId, asset->getShaderBegin(), asset->getShaderEnd());
  m_usesInstanceData = !instanceBindings.empty();
  if (m_usesInstanceData) {
    const auto& binding = *instanceBindings.begin();
    if (binding.first != 0U || binding.second != DescriptorBindingKind::UniformBufferDynamic) {
      throw err::GraphicErr{m_asset->getId(), "Invalid global binding"};
    }
  }
}

Graphic::~Graphic() {
  if (m_vkPipeline) {
    vkDestroyPipelineLayout(m_device->getVkDevice(), m_vkPipelineLayout, nullptr);
    vkDestroyPipeline(m_device->getVkDevice(), m_vkPipeline, nullptr);
  }
}

auto Graphic::prepareResources(
    Transferer* transferer,
    UniformContainer* uni,
    VkRenderPass vkRenderPass,
    VkSampleCount sampleCount) const -> void {

  if (m_mesh) {
    m_mesh->prepareResources(transferer);
  }
  for (const auto& texData : m_textures) {
    texData.texture->prepareResources(transferer);
  }

  if (!m_vkPipeline) {

    // Data for both the global and instance descriptors come from the uniform container.
    m_vkPipelineLayout = createPipelineLayout(
        m_device, uni->getVkDescLayout(), m_descSet.getVkLayout(), uni->getVkDescLayout());
    m_vkPipeline = createPipeline(
        m_device,
        vkRenderPass,
        sampleCount,
        m_vkPipelineLayout,
        m_shaders,
        m_asset->getVertexTopology(),
        m_asset->getRasterizerMode(),
        m_asset->getLineWidth(),
        m_asset->getBlendMode(),
        m_asset->getDepthTestMode(),
        m_asset->getCullMode());

    DBG_PIPELINELAYOUT_NAME(m_device, m_vkPipelineLayout, m_asset->getId());
    DBG_PIPELINE_NAME(m_device, m_vkPipeline, m_asset->getId());

    LOG_D(m_logger, "Vulkan pipline created", {"asset", m_asset->getId()});
  }
}

} // namespace tria::gfx::internal

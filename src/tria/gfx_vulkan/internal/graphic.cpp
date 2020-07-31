#include "graphic.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include <array>
#include <cassert>

namespace tria::gfx::internal {

namespace {

[[nodiscard]] auto createVkDescriptorSetLayout(const Device* device, uint32_t texCount)
    -> VkDescriptorSetLayout {

  std::vector<VkDescriptorSetLayoutBinding> bindings =
      std::vector<VkDescriptorSetLayoutBinding>{texCount};
  for (auto i = 0U; i != texCount; ++i) {
    bindings[i].binding         = i;
    bindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[i].descriptorCount = 1U;
    bindings[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount                    = texCount;
  layoutInfo.pBindings                       = bindings.data();

  VkDescriptorSetLayout result;
  checkVkResult(vkCreateDescriptorSetLayout(device->getVkDevice(), &layoutInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto createVkDescriptorPool(const Device* device, uint32_t texCount)
    -> VkDescriptorPool {
  VkDescriptorPoolSize poolSize = {};
  poolSize.type                 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSize.descriptorCount      = std::max(1U, texCount);

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount              = 1U;
  poolInfo.pPoolSizes                 = &poolSize;
  poolInfo.maxSets                    = std::max(1U, texCount);

  VkDescriptorPool result;
  checkVkResult(vkCreateDescriptorPool(device->getVkDevice(), &poolInfo, nullptr, &result));
  return result;
}

[[nodiscard]] auto allocVkDescriptorSet(
    const Device* device, const VkDescriptorPool pool, const VkDescriptorSetLayout layout)
    -> VkDescriptorSet {

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool              = pool;
  allocInfo.descriptorSetCount          = 1U;
  allocInfo.pSetLayouts                 = &layout;

  VkDescriptorSet result;
  checkVkResult(vkAllocateDescriptorSets(device->getVkDevice(), &allocInfo, &result));
  return result;
}

auto configureVkDescriptorSet(
    const Device* device,
    VkDescriptorSet descSet,
    uint32_t dstBinding,
    const Image& img,
    const ImageSampler& sampler) -> void {

  VkDescriptorImageInfo imgInfo = {};
  imgInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imgInfo.imageView             = img.getVkImageView();
  imgInfo.sampler               = sampler.getVkSampler();

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet               = descSet;
  descriptorWrite.dstBinding           = dstBinding;
  descriptorWrite.dstArrayElement      = 0U;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount      = 1U;
  descriptorWrite.pImageInfo           = &imgInfo;

  vkUpdateDescriptorSets(device->getVkDevice(), 1U, &descriptorWrite, 0U, nullptr);
}

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
    const Mesh* mesh) {

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
  colorBlendAttachment.blendEnable = false;

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
    const Device* device,
    const asset::Graphic* asset,
    AssetResource<Shader>* shaders,
    AssetResource<Mesh>* meshes,
    AssetResource<Texture>* textures) :
    m_logger{logger}, m_device{device}, m_asset{asset}, m_vkPipeline{nullptr} {
  assert(m_device);
  assert(m_asset);

  m_vkDescLayout = createVkDescriptorSetLayout(device, asset->getTextureCount());
  m_vkDescPool   = createVkDescriptorPool(device, asset->getTextureCount());
  m_vkDescSet    = allocVkDescriptorSet(device, m_vkDescPool, m_vkDescLayout);

  m_vertShader = shaders->get(m_asset->getVertShader());
  m_fragShader = shaders->get(m_asset->getFragShader());
  m_mesh       = meshes->get(m_asset->getMesh());

  m_textures.reserve(m_asset->getTextureCount());
  auto dstBinding = 0U;
  for (auto itr = m_asset->getTextureBegin(); itr != m_asset->getTextureEnd(); ++itr) {
    const auto* tex = textures->get(*itr);
    auto sampler    = ImageSampler{device};
    configureVkDescriptorSet(device, m_vkDescSet, dstBinding++, tex->getImage(), sampler);
    m_textures.emplace_back(tex, std::move(sampler));
  }
}

Graphic::~Graphic() {
  if (m_vkPipeline) {
    vkDestroyPipelineLayout(m_device->getVkDevice(), m_vkPipelineLayout, nullptr);
    vkDestroyPipeline(m_device->getVkDevice(), m_vkPipeline, nullptr);
  }
  vkDestroyDescriptorSetLayout(m_device->getVkDevice(), m_vkDescLayout, nullptr);
  vkDestroyDescriptorPool(m_device->getVkDevice(), m_vkDescPool, nullptr);
}

auto Graphic::prepareResources(
    Transferer* transferer, UniformContainer* uni, VkRenderPass vkRenderPass) const -> void {

  m_mesh->prepareResources(transferer);
  for (const auto& texData : m_textures) {
    texData.texture->prepareResources(transferer);
  }

  if (!m_vkPipeline) {
    m_vkPipelineLayout =
        createPipelineLayout<2>(m_device->getVkDevice(), {m_vkDescLayout, uni->getVkDescLayout()});
    m_vkPipeline = createPipeline(
        m_device->getVkDevice(),
        vkRenderPass,
        m_vkPipelineLayout,
        m_vertShader,
        m_fragShader,
        m_mesh);

    LOG_D(
        m_logger,
        "Vulkan pipline created",
        {"asset", m_asset->getId()},
        {"texCount", m_textures.size()});
  }
}

} // namespace tria::gfx::internal

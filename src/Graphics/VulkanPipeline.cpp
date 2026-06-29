/*****************************************************
    Includes
*****************************************************/
#include "Graphics/VulkanPipeline.h"
#include "Graphics/VulkanTypes.h"
#include <stdexcept>
#include <array>

VulkanPipeline::VulkanPipeline()
{
    // Init empty pipeline
    Clear();
}

void VulkanPipeline::Clear()
{
    // Init all structs
    mShaderStages.clear();
    mInputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    mRasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    mColorBlendAttachment = {};
    mMultisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    mPipelineLayout = {};
    mDepthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    mRenderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
}

void VulkanPipeline::BuildPipeline(VkDevice const& _device, std::vector<VkDynamicState> const& _dynamicStates)
{
    // Begin pipeline creation
    // Specify dynamic states for the pipeline
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());
    dynamicState.pDynamicStates = _dynamicStates.data();

    // Set vertex input binding
    VkVertexInputBindingDescription vertexInputBinding{};
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Set vertex input attribs
    std::array<VkVertexInputAttributeDescription, 4> vertexInputAttribs{};
    // Position
    vertexInputAttribs[0].binding = 0;
    vertexInputAttribs[0].location = 0;
    vertexInputAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttribs[0].offset = offsetof(Vertex, position);
    // UV
    vertexInputAttribs[1].binding = 0;
    vertexInputAttribs[1].location = 1;
    vertexInputAttribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttribs[1].offset = offsetof(Vertex, uv);
    // Normal
    vertexInputAttribs[2].binding = 0;
    vertexInputAttribs[2].location = 2;
    vertexInputAttribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttribs[2].offset = offsetof(Vertex, normal);
    // Tangent
    vertexInputAttribs[3].binding = 0;
    vertexInputAttribs[3].location = 3;
    vertexInputAttribs[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexInputAttribs[3].offset = offsetof(Vertex, tangent);

    // Specify format of vertex data to be passed to vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttribs.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttribs.data();

    // Init viewport state create info
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Setup rasterizer
    mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    mRasterizer.depthClampEnable = VK_FALSE;
    mRasterizer.rasterizerDiscardEnable = VK_FALSE;
    mRasterizer.depthBiasEnable = VK_FALSE;

    // Init color blending create info
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &mColorBlendAttachment;

    // Init pipeline create info
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = mShaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &mInputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &mRasterizer;
    pipelineInfo.pMultisampleState = &mMultisampling;
    pipelineInfo.pDepthStencilState = &mDepthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.pNext = &mRenderInfo;

    // Create pipeline
    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanPipeline::Destroy(VkDevice const& _device)
{
    // Destroy pipeline first
    vkDestroyPipeline(_device, mPipeline, nullptr);

    // Destroy pipeline layout next
    vkDestroyPipelineLayout(_device, mPipelineLayout, nullptr);
}

VkPipeline& VulkanPipeline::GetPipeline(void)
{
    return mPipeline;
}

VkPipelineLayout& VulkanPipeline::GetPipelineLayout(void)
{
    return mPipelineLayout;
}

void VulkanPipeline::SetShaderStages(VkPipelineShaderStageCreateInfo const& _vert, VkPipelineShaderStageCreateInfo const& _frag)
{
    mShaderStages.push_back(_vert);
    mShaderStages.push_back(_frag);
}

void VulkanPipeline::SetInputAssembly(VkPrimitiveTopology _topo)
{
    mInputAssembly.topology = _topo;
    mInputAssembly.primitiveRestartEnable = VK_FALSE; // Disable primitive restart
}

void VulkanPipeline::DisableMultisampling(void)
{
    // Setup multisampling info (disabled for now)
    mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mMultisampling.sampleShadingEnable = VK_FALSE;
    mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    mMultisampling.minSampleShading = 1.f;
    mMultisampling.pSampleMask = nullptr;
    mMultisampling.alphaToCoverageEnable = VK_FALSE;
    mMultisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanPipeline::SetPolygonMode(VkPolygonMode _mode)
{
    mRasterizer.polygonMode = _mode;
    mRasterizer.lineWidth = 1.f;
}

void VulkanPipeline::SetCullMode(Material::CULL_MODE _mode)
{
    // Set cull mode
    if (_mode == Material::CM_CULL_NONE) {
        mRasterizer.cullMode = VK_CULL_MODE_NONE;
    }
    else if (_mode == Material::CM_CULL_BACK) {
        mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    }
    else if (_mode == Material::CM_CULL_FRONT) {
        mRasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    }
    else {
        mRasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
    }

    // Set default front face
    mRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
}

void VulkanPipeline::EnableDepthTest(bool _depthWriteEnable, VkCompareOp _op)
{
    mDepthStencil.depthTestEnable = VK_TRUE;
    mDepthStencil.depthWriteEnable = _depthWriteEnable;
    mDepthStencil.depthCompareOp = _op;
    mDepthStencil.depthBoundsTestEnable = VK_FALSE;
    mDepthStencil.stencilTestEnable = VK_FALSE;
    mDepthStencil.front = {};
    mDepthStencil.back = {};
    mDepthStencil.minDepthBounds = 0.f;
    mDepthStencil.maxDepthBounds = 1.f;
}

void VulkanPipeline::DisableDepthTest(void)
{
    mDepthStencil.depthTestEnable = VK_FALSE;
    mDepthStencil.depthWriteEnable = VK_FALSE;
    mDepthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    mDepthStencil.depthBoundsTestEnable = VK_FALSE;
    mDepthStencil.stencilTestEnable = VK_FALSE;
    mDepthStencil.front = mDepthStencil.back = {};
    mDepthStencil.minDepthBounds = 0.f;
    mDepthStencil.maxDepthBounds = 1.f;
}

void VulkanPipeline::SetColorAttachmentFormat(VkFormat _format)
{
    mColorAttachmentformat = _format;
    mRenderInfo.colorAttachmentCount = 1;
    mRenderInfo.pColorAttachmentFormats = &mColorAttachmentformat;
}

void VulkanPipeline::SetDepthFormat(VkFormat _format)
{
    mRenderInfo.depthAttachmentFormat = _format;
}

void VulkanPipeline::DisableBlending(void)
{
    // Set color blending info
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // Disable blending
    mColorBlendAttachment.blendEnable = VK_FALSE;
}

void VulkanPipeline::EnableAdditiveBlending(void)
{
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    mColorBlendAttachment.blendEnable = VK_TRUE;
    mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void VulkanPipeline::EnableAlphaBlending(void)
{
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    mColorBlendAttachment.blendEnable = VK_TRUE;
    mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void VulkanPipeline::EnablePreMultipliedAlphaBlending(void)
{
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    mColorBlendAttachment.blendEnable = VK_TRUE;
    mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

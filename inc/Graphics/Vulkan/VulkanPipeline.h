#ifndef VULKAN_PIPELINE
#define VULKAN_PIPELINE

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"
#include <vector>
#include <Graphics/Material.h>

class VulkanPipeline 
{
public:
    // Pipeline base functions
    VulkanPipeline();
    void Clear();
    void BuildPipeline(VkDevice const& _device, std::vector<VkDynamicState> const& _dynamicStates);
    void Destroy(VkDevice const& _device);
    VkPipeline& GetPipeline(void);
    VkPipelineLayout& GetPipelineLayout(void);

    // Functions to set pipeline create infos
    void SetShaderStages(VkPipelineShaderStageCreateInfo const& _vert, VkPipelineShaderStageCreateInfo const& _frag);
    void SetInputAssembly(VkPrimitiveTopology _topo);
    void DisableMultisampling(void);
    void SetPolygonMode(VkPolygonMode _mode);
    void SetCullMode(Material::CULL_MODE _mode);
    void EnableDepthTest(bool _depthWriteEnable, VkCompareOp _op);
    void DisableDepthTest(void);
    void SetColorAttachmentFormat(VkFormat _format);
    void SetDepthFormat(VkFormat _format);

    // For blending
    void DisableBlending(void);
    void EnableAdditiveBlending(void);
    void EnableAlphaBlending(void);
    void EnablePreMultipliedAlphaBlending(void);
private:
    std::vector<VkPipelineShaderStageCreateInfo> mShaderStages;
    VkPipelineInputAssemblyStateCreateInfo mInputAssembly;
    VkPipelineRasterizationStateCreateInfo mRasterizer;
    VkPipelineColorBlendAttachmentState mColorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo mMultisampling;
    VkPipelineDepthStencilStateCreateInfo mDepthStencil;
    VkPipelineRenderingCreateInfo mRenderInfo;
    VkFormat mColorAttachmentformat;

    // Pipeline
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;
};

#endif
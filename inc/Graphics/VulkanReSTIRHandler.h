#ifndef RESTIR_H
#define RESTIR_H

/*****************************************************
	Includes
*****************************************************/
#include <Graphics/VulkanTypes.h>
#include <Graphics/VulkanDescriptorPool.h>

struct Reservoir
{
    unsigned mostImptLightIdx;
    float lightWeight;
    float sumLightWeight;
    float numLightsProcessed;
};

class VulkanReSTIRHandler
{
public:
	void CreateReSTIRBuffers(VkDevice const& _logicalDevice, VmaAllocator& _allocator, glm::uvec2 const& _renderDims);
	void DestroyReSTIRResources(VkDevice const& _logicalDevice, VmaAllocator& _allocator);
    void BindReSTIRBuffers(VkDevice const& _logicalDevice);
    void PostRenderSwapBuffers(void);

    // Reservoir buffers
    VkDeviceSize mReservoirBufferSize{};
    AllocatedBuffer mCurrReservoirBuffer{};
    AllocatedBuffer mPrevReservoirBuffer{};

    // Normals and velocity buffer for temporal reuse
    AllocatedImage mCurrNormalsBuffer{};
    AllocatedImage mPrevNormalsBuffer{};
    AllocatedImage mCurrVelocityBuffer{};

    // Descriptor set layout for ReSTIR
    VkDescriptorSetLayout mReSTIRDescriptorSetLayout{};
    VkDescriptorSet mReSTIRDescriptor{};
};

#endif

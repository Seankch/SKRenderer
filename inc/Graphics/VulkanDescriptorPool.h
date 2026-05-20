#ifndef VULKAN_DESCRIPTOR_POOL
#define VULKAN_DESCRIPTOR_POOL

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"
#include <vector>
#include <span>

// Descriptor Allocator Growable
class VulkanDescriptorPool
{
public:
    struct PoolSizeRatio 
    {
        VkDescriptorType type;
        float ratio;
    };

    void InitPool(VkDevice _device, uint32_t _initialSets, std::span<PoolSizeRatio> _poolRatios);
    void ClearPools(VkDevice _device);
    void DestroyPool(VkDevice _device);
    VkDescriptorSet Allocate(VkDevice _device, VkDescriptorSetLayout _layout, void* _pNext = nullptr);

private:
    VkDescriptorPool GetPool(VkDevice _device);
    VkDescriptorPool CreatePool(VkDevice _device, uint32_t _setCount, std::span<PoolSizeRatio> _poolRatios);

    std::vector<PoolSizeRatio> mRatios;
    std::vector<VkDescriptorPool> mFullPools;
    std::vector<VkDescriptorPool> mReadyPools;
    uint32_t mSetsPerPool{};
};

#endif

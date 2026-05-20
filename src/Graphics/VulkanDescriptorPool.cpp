#include "Graphics/VulkanDescriptorPool.h"

void VulkanDescriptorPool::InitPool(VkDevice _device, uint32_t _maxSets, std::span<PoolSizeRatio> _poolRatios)
{
    // Set ratios
    mRatios.clear();
    for (auto ratio : _poolRatios) 
    {
        mRatios.push_back(ratio);
    }

    // Allocate descriptor pool and add it to ready pool list
    VkDescriptorPool newPool = CreatePool(_device, _maxSets, _poolRatios);
    mSetsPerPool = _maxSets * 2;
    mReadyPools.push_back(newPool);
}

void VulkanDescriptorPool::ClearPools(VkDevice _device)
{
    for (auto pool : mReadyPools) 
    {
        vkResetDescriptorPool(_device, pool, 0);
    }
    for (auto pool : mFullPools)
    {
        vkResetDescriptorPool(_device, pool, 0);
        mReadyPools.push_back(pool);
    }
    mFullPools.clear();
}

void VulkanDescriptorPool::DestroyPool(VkDevice _device)
{
    // Destroy descriptor pool
    for (auto pool : mReadyPools)
    {
        vkDestroyDescriptorPool(_device, pool, nullptr);
    }
    mReadyPools.clear();
    for (auto pool : mFullPools)
    {
        vkDestroyDescriptorPool(_device, pool, nullptr);
    }
    mFullPools.clear();
}

VkDescriptorSet VulkanDescriptorPool::Allocate(VkDevice _device, VkDescriptorSetLayout _layout, void* _pNext)
{
    // Get or create pool
    VkDescriptorPool poolToUse = GetPool(_device);

    // Allocate descriptor sets
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = _pNext;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = poolToUse;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_layout;
    VkDescriptorSet descSet;
    VkResult result = vkAllocateDescriptorSets(_device, &allocInfo, &descSet);

    // Check for failed allocation due to OOM or frag issue
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) 
    {
        // Add to full pools array
        mFullPools.push_back(poolToUse);
        poolToUse = GetPool(_device);
        allocInfo.descriptorPool = poolToUse;
        vkAllocateDescriptorSets(_device, &allocInfo, &descSet);
    }

    // Add to ready pool
    mReadyPools.push_back(poolToUse);
    return descSet;
}

VkDescriptorPool VulkanDescriptorPool::GetPool(VkDevice _device)
{
    VkDescriptorPool newPool;
    if (mReadyPools.size() != 0) 
    {
        // Fetch from ready pool list
        newPool = mReadyPools.back();
        mReadyPools.pop_back();
    }
    else 
    {
        // Create a new pool
        newPool = CreatePool(_device, mSetsPerPool, mRatios);
        mSetsPerPool = mSetsPerPool * 2;
        if (mSetsPerPool > 4092) 
        {
            mSetsPerPool = 4092;
        }
    }

    return newPool;
}

VkDescriptorPool VulkanDescriptorPool::CreatePool(VkDevice _device, uint32_t _setCount, std::span<PoolSizeRatio> _poolRatios)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : _poolRatios) 
    {
        poolSizes.push_back(VkDescriptorPoolSize
        {
            .type = ratio.type,
            .descriptorCount = uint32_t(ratio.ratio * _setCount)
        });
    }

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    pool_info.maxSets = _setCount;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    VkDescriptorPool newPool;
    vkCreateDescriptorPool(_device, &pool_info, nullptr, &newPool);
    return newPool;
}



#ifndef VULKAN_DESCRIPTOR
#define VULKAN_DESCRIPTOR

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"
#include <vector>
#include <span>

// DescriptorLayoutBuilder
class VulkanDescriptor
{
public:
    void AddBinding(uint32_t _binding, VkDescriptorType _type, uint32_t _descCount = 1);
    void Clear();
    VkDescriptorSetLayout Build(VkDevice _device, VkShaderStageFlags _shaderStages, void* _pNext = nullptr, VkDescriptorSetLayoutCreateFlags _flags = 0);

private:
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

#endif

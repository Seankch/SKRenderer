#include "Graphics/VulkanDescriptor.h"

void VulkanDescriptor::AddBinding(uint32_t _binding, VkDescriptorType _type, uint32_t _descCount)
{
    VkDescriptorSetLayoutBinding newbind{};
    newbind.binding = _binding;
    newbind.descriptorCount = _descCount;
    newbind.descriptorType = _type;
    bindings.push_back(newbind);
}

void VulkanDescriptor::Clear()
{
    bindings.clear();
}

VkDescriptorSetLayout VulkanDescriptor::Build(VkDevice _device, VkShaderStageFlags _shaderStages, void* _pNext, VkDescriptorSetLayoutCreateFlags _flags)
{
    for (auto& b : bindings) 
    {
        b.stageFlags |= _shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pNext = _pNext;
    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = _flags;

    VkDescriptorSetLayout set;
    vkCreateDescriptorSetLayout(_device, &info, nullptr, &set);
    return set;
}

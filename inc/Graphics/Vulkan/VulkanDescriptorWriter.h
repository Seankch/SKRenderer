#ifndef VULKAN_DESCRIPTOR_WRITER
#define VULKAN_DESCRIPTOR_WRITER

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"
#include <vector>
#include <deque>

// Descriptor Writer
class VulkanDescriptorWriter
{
public:
    void WriteImage(int _binding, VkImageView _image, VkSampler _sampler, VkImageLayout _layout, VkDescriptorType _type, uint32_t _dstArrayElement = 0);
    void WriteBuffer(int _binding, VkBuffer _buffer, size_t _size, size_t _offset, VkDescriptorType _type);
    void WriteAccelerationStructure(int _binding, VkAccelerationStructureKHR* _tlas);
    void Clear();
    void UpdateSet(VkDevice _device, VkDescriptorSet _set);

private:
    std::deque<VkDescriptorImageInfo> imageInfos;
    std::deque<VkDescriptorBufferInfo> bufferInfos;
    std::deque<VkWriteDescriptorSetAccelerationStructureKHR> asInfos;
    std::vector<VkWriteDescriptorSet> writes;
};

#endif
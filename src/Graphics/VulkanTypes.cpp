#include "Graphics/VulkanTypes.h"
#include "Graphics/VulkanUtils.h"
#include <stdexcept>

void AllocatedBuffer::CreateBuffer(VmaAllocator& _allocator, VkDeviceSize _allocSize, VkBufferUsageFlags _usage, VmaMemoryUsage _memoryUsage)
{
    // Allocate buffer
    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.pNext = nullptr;
    bufferInfo.size = _allocSize;
    bufferInfo.usage = _usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = _memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    // Allocate buffer
    VkResult res = vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, &info);
    if (res != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    // Init buffer variables
    size = _allocSize;
    capacity = _allocSize;
}

void AllocatedBuffer::DestroyBuffer(VmaAllocator& _allocator)
{
    vmaDestroyBuffer(_allocator, buffer, allocation);
}

void AllocatedImage::CreateImage(VkDevice const& _logicalDevice, VmaAllocator& _allocator, VkExtent3D _size, VkFormat _format, VkImageUsageFlags _usage, bool _mipmapped)
{
    imageFormat = _format;
    imageExtent = _size;

    // Set mipmap levels
    VkImageCreateInfo imgInfo = VulkanUtils::GetImageCreateInfo(_format, _usage, _size);
    if (_mipmapped)
    {
        imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_size.width, _size.height)))) + 1;
    }

    // Allocate images on dedicated GPU memory
    VmaAllocationCreateInfo allocinfo = {};
    allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create the image
    vmaCreateImage(_allocator, &imgInfo, &allocinfo, &image, &allocation, nullptr);

    // If format is depth format, set depth bit aspect flag
    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (_format == VK_FORMAT_D32_SFLOAT)
    {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    // Build imageview for the image
    VkImageViewCreateInfo viewInfo = VulkanUtils::GetImageViewCreateInfo(_format, image, aspectFlag);
    viewInfo.subresourceRange.levelCount = imgInfo.mipLevels;
    vkCreateImageView(_logicalDevice, &viewInfo, nullptr, &imageView);
}

void AllocatedImage::DestroyImage(VkDevice const& _logicalDevice, VmaAllocator& _allocator)
{
    vkDestroyImageView(_logicalDevice, imageView, nullptr);
    vmaDestroyImage(_allocator, image, allocation);
}

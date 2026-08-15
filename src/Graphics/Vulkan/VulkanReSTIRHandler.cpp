#include "Graphics/Vulkan/VulkanReSTIRHandler.h"
#include "Graphics/Vulkan/VulkanDescriptor.h"

void VulkanReSTIRHandler::CreateReSTIRBuffers(VkDevice const& _logicalDevice, VmaAllocator& _allocator, glm::uvec2 const& _renderDims)
{
    // Create reservoir buffers
    mReservoirBufferSize = _renderDims.x * _renderDims.y * sizeof(Reservoir);
    mCurrReservoirBuffer.CreateBuffer(_allocator, mReservoirBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    mPrevReservoirBuffer.CreateBuffer(_allocator, mReservoirBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    // Create image buffers
    VkExtent3D renderExtent = { _renderDims.x, _renderDims.y, 1 };
    mCurrNormalsBuffer.CreateImage(_logicalDevice, _allocator, renderExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    mPrevNormalsBuffer.CreateImage(_logicalDevice, _allocator, renderExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    mCurrVelocityBuffer.CreateImage(_logicalDevice, _allocator, renderExtent, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
}

void VulkanReSTIRHandler::DestroyReSTIRResources(VkDevice const& _logicalDevice, VmaAllocator& _allocator)
{
    // Destroy reservoir buffers
    mCurrReservoirBuffer.DestroyBuffer(_allocator);
    mPrevReservoirBuffer.DestroyBuffer(_allocator);

    // Destroy image buffers
    mCurrNormalsBuffer.DestroyImage(_logicalDevice, _allocator);
    mPrevNormalsBuffer.DestroyImage(_logicalDevice, _allocator);
    mCurrVelocityBuffer.DestroyImage(_logicalDevice, _allocator);

    // Destroy descriptor set layout
    vkDestroyDescriptorSetLayout(_logicalDevice, mReSTIRDescriptorSetLayout, nullptr);
}

void VulkanReSTIRHandler::BindReSTIRBuffers(VkDevice const& _logicalDevice)
{
    // Init descriptor for scene data
    VulkanDescriptor desc{};
    desc.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);         // Curr reservoir buffer
    desc.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);         // Prev reservoir buffer
    desc.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // Curr normals buffer
    desc.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // Prev normals buffer
    desc.AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // Curr velocity buffer
    desc.AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // Depth buffer
    mReSTIRDescriptorSetLayout = desc.Build(_logicalDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanReSTIRHandler::PostRenderSwapBuffers(void)
{
    std::swap(mCurrReservoirBuffer, mPrevReservoirBuffer);
    std::swap(mCurrNormalsBuffer, mPrevNormalsBuffer);
}

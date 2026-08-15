#ifndef VULKAN_UTILS
#define VULKAN_UTILS

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"

namespace VulkanUtils
{
    // For getting subresource range
    inline VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags const& _aspectMask)
    {
        VkImageSubresourceRange subImage{};
        subImage.aspectMask = _aspectMask;
        subImage.baseMipLevel = 0;
        subImage.levelCount = VK_REMAINING_MIP_LEVELS;
        subImage.baseArrayLayer = 0;
        subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
        return subImage;
    }

    // For transitioning image between presentable and writable mode
    inline void TransitionImage(VkCommandBuffer const& _cmd, VkImage const& _img, VkImageLayout const& _currLayout, VkImageLayout const& _newLayout)
    {
        // Init barrier create info
        VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        imageBarrier.pNext = nullptr;
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
        imageBarrier.oldLayout = _currLayout;
        imageBarrier.newLayout = _newLayout;

        // Init aspect mask
        VkImageAspectFlags aspectMask = (_newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange = GetImageSubresourceRange(aspectMask);
        imageBarrier.image = _img;

        // Init dependency info
        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &imageBarrier;

        // Create pipeline barrier
        vkCmdPipelineBarrier2(_cmd, &dependencyInfo);
    }

    // For submitting semaphore info
    inline VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 const& stageMask, VkSemaphore const& semaphore)
    {
        VkSemaphoreSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.semaphore = semaphore;
        submitInfo.stageMask = stageMask;
        submitInfo.deviceIndex = 0;
        submitInfo.value = 1;
        return submitInfo;
    }

    // For submitting command buffer info
    inline VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer const& cmd)
    {
        VkCommandBufferSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        info.pNext = nullptr;
        info.commandBuffer = cmd;
        info.deviceMask = 0;
        return info;
    }

    // For submitting cbo and semaphore info
    inline VkSubmitInfo2 GetSubmitInfo(VkCommandBufferSubmitInfo const* cmd, VkSemaphoreSubmitInfo const* signalSemaphoreInfo, VkSemaphoreSubmitInfo const* waitSemaphoreInfo)
    {
        VkSubmitInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;
        info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
        info.pWaitSemaphoreInfos = waitSemaphoreInfo;
        info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
        info.pSignalSemaphoreInfos = signalSemaphoreInfo;
        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = cmd;
        return info;
    }

    inline VkImageCreateInfo GetImageCreateInfo(VkFormat const& _format, VkImageUsageFlags const& _usageFlags, VkExtent3D const& _extent)
    {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = _format;
        info.extent = _extent;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = _usageFlags;
        return info;
    }

    inline VkImageViewCreateInfo GetImageViewCreateInfo(VkFormat const& _format, VkImage const& _image, VkImageAspectFlags const& _aspectFlags)
    {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = _image;
        info.format = _format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = _aspectFlags;
        return info;
    }

    inline VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo(void)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        return pipelineLayoutInfo;
    }

    inline void CopyImageToImage(VkCommandBuffer const& _cmd, VkImage const& _src, VkImage const& _dest, VkExtent2D const& _srcSize, VkExtent2D const& _dstSize)
    {
        // Init blit region
        VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

        // Set src and dest offsets
        blitRegion.srcOffsets[1].x = _srcSize.width;
        blitRegion.srcOffsets[1].y = _srcSize.height;
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[1].x = _dstSize.width;
        blitRegion.dstOffsets[1].y = _dstSize.height;
        blitRegion.dstOffsets[1].z = 1;

        // Set src and dest sub resources
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstSubresource.mipLevel = 0;

        // Set blit info
        VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
        blitInfo.dstImage = _dest;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.srcImage = _src;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        // Perform blit command
        vkCmdBlitImage2(_cmd, &blitInfo);
    }

    // For attachment info
    inline VkRenderingAttachmentInfo GetAttachmentInfo(VkImageView const& _imgView, VkClearValue const* _clearVal, VkImageLayout const& _layout)
    {
        // Set and return color attachment
        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = _imgView;
        colorAttachment.imageLayout = _layout;
        colorAttachment.loadOp = _clearVal? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (_clearVal) {
            colorAttachment.clearValue = *_clearVal;
        }
        return colorAttachment;
    }

    // For depth attachment info
    inline VkRenderingAttachmentInfo GetDepthAttachmentInfo(VkImageView const& _imgView, VkImageLayout const& _layout, VkAttachmentLoadOp _loadOp)
    {
        VkRenderingAttachmentInfo depthAttachment{}; 
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; 
        depthAttachment.imageView = _imgView; 
        depthAttachment.imageLayout = _layout; 
        depthAttachment.loadOp = _loadOp;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 
        depthAttachment.clearValue.depthStencil.depth = 1.0f; 
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 }; 
        return depthAttachment;
    }

    // For render info
    inline VkRenderingInfo GetRenderInfo(VkExtent2D const& _renderExtent, VkRenderingAttachmentInfo const* _colorAttachment, VkRenderingAttachmentInfo const* _depthAttachment)
    {
        // Init and return render info
        VkRenderingInfo renderInfo{};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, _renderExtent };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = _colorAttachment;
        renderInfo.pDepthAttachment = _depthAttachment;
        renderInfo.pStencilAttachment = nullptr;
        return renderInfo;
    }
}

#endif
/*****************************************************
    Includes
*****************************************************/
#include "Graphics/VulkanRenderer.h"
#include "Graphics/VulkanUtils.h"
#include "Graphics/VulkanDescriptorPool.h"
#include "Graphics/VulkanDescriptor.h"
#include "Graphics/VulkanDescriptorWriter.h"
#include "Graphics/VulkanShader.h"
#include "Managers/WindowsManager.h"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include <fstream>
#include <set>
#include <iostream>
#include <map>

// ImGui includes
#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#define IMGUI_IMPL_VULKAN_LOADER
#include <imgui_impl_vulkan.h>

// VMA includes
#define VMA_IMPLEMENTATION
#include "Vulkan/Include/vma/vk_mem_alloc.h"

VulkanRenderer::VulkanRenderer(WindowsManager* _wm)
  : mInst{}, mPhysicalDevice{ VK_NULL_HANDLE }, mLogicalDevice{}, mEnableValidationLayers{}, mMinUboOffsetAlignment{},
  mSurface{}, mSwapChain{}, mGraphicsQueue{}, mPresentQueue{}, mSwapChainImageFormat{}, 
  mSwapChainExtent{}, windowsMgr{ _wm }
{
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::Init(void)
{
    // Set validation layers
    mValidationLayers = 
    {
        "VK_LAYER_KHRONOS_validation"
    };

    // Set required device extensions
    mDeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,                // Enable swapchain
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,        // Enable synchronization2
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,        // Enable dynamic rendering
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,    // Enable BDA
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,      // Enable descriptor indexing
        // Ray tracing extensions
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, // Enable deferred host operations
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,   // Enable acceleration structure
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,     // Enable ray tracing pipeline
        VK_KHR_RAY_QUERY_EXTENSION_NAME                 // Enable ray query
    };

    // Set default dynamic states for pipeline
    mDynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,          // Enable changing of viewport size after pipeline is created
        VK_DYNAMIC_STATE_SCISSOR,           // Enable updating of scissor mode
        //VK_DYNAMIC_STATE_CULL_MODE,       // Enable updating of cull mode
        //VK_DYNAMIC_STATE_BLEND_CONSTANTS, // Enable updating of blend modes
        //VK_DYNAMIC_STATE_LINE_WIDTH,      // Enable updating of line width
        //VK_DYNAMIC_STATE_POLYGON_MODE_EXT // Enable updating of polygon mode
    };

    // Enable/disable validation layers
#ifdef _DEBUG
    mEnableValidationLayers = true;
#else
    mEnableValidationLayers = false;
#endif

    try
    {
        // Create VK instance
        CreateInstance();

        // Create surface
        CreateSurface();

        // Pick physical device (Choose which GPU to use)
        PickPhysicalDevice();

        // Create logical device
        CreateLogicalDevice();

        // Load extension functions
        mVkLoader.LoadFunctions(mLogicalDevice);

        // Create swap chain
        CreateSwapChain();

        // Create image views
        CreateImageViews();

        // Create command pool
        CreateCommandPoolAndBuffer();

        // Create sync objects
        CreateSyncObjects();

        // Init memory allocator
        InitMemoryAllocator();

        // Init framebuffers
        InitFrameBuffers();

        // Init texture samplers
        InitDefaultSamplers();

        // Init descriptor pools
        InitDescriptorPools();
    }
    catch (std::exception const& _e)
    {
        // Display error msg
        std::cout << "Error: " << _e.what() << std::endl;

        // TODO: maybe switch to opengl
    }
}

void VulkanRenderer::LateInit(void)
{
    try
    {
        // Init descriptors
        InitDescriptors();
    }
    catch (std::exception const& _e)
    {
        // Display error msg
        std::cout << "Error: " << _e.what() << std::endl;

        // TODO: maybe switch to opengl
    }
}

void VulkanRenderer::ClearBuffer()
{
    // Get command buffer
    VkCommandBuffer& cmd = GetCurrFrame().commandBuffer;

    // Set vk clear color
    VkClearColorValue clearVal{};
    clearVal = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };

    // Set clear range
    VkImageSubresourceRange clearRange = VulkanUtils::GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    // Clear image
    vkCmdClearColorImage(cmd, mDrawImg->image, VK_IMAGE_LAYOUT_GENERAL, &clearVal, 1, &clearRange);
}

void VulkanRenderer::Render(Model::Mesh const& _mesh, Material& _mat, glm::mat4 const& _modelXForm, bool _hasPreMultipliedAlpha, bool _isFirstObject)
{
    // Get current frame
    FrameData& frame = GetCurrFrame();
    VkCommandBuffer& cmd = frame.commandBuffer;

    // Bind pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineMap[_mat.GetName()].GetPipeline());

    // Set viewport info
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(mDrawExtent->width);
    viewport.height = static_cast<float>(mDrawExtent->height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    // Set scissors info
    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = mDrawExtent->width;
    scissor.extent.height = mDrawExtent->height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Get descriptor sets
    VulkanMaterialData& matData = mMatMap[_mat.GetName()];
    VkDescriptorSet sets[] = 
    {
        mGlobalDescriptor,                // set 0, GPUSceneData
        mBindlessTextureDescriptor,       // set 1, bindless textures
        mReSTIRHandler.mReSTIRDescriptor, // set 2, ReSTIR buffers
        matData.descriptorSet             // set 3, material UBO
    };

    // Bind descriptor sets
    vkCmdBindDescriptorSets(cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        mPipelineMap[_mat.GetName()].GetPipelineLayout(),
        0,                                  
        static_cast<uint32_t>(std::size(sets)),
        sets,
        0,
        nullptr);

    // Set push constants
    GPUDrawPushConstants pushConstants;
    pushConstants.worldMatrix = _modelXForm;
    pushConstants.vertexBuffer = _mesh.meshBuffer.vertexBufferAddress;
    vkCmdPushConstants(cmd, mPipelineMap[_mat.GetName()].GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
    
    // Bind index buffer
    vkCmdBindIndexBuffer(cmd, _mesh.meshBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // Draw object and end rendering
    vkCmdDrawIndexed(cmd, _mesh.indicesCount, 1, 0, 0, 0);
}

void VulkanRenderer::Exit()
{
    // Destroy command pool
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        // Destroy command pool
        vkDestroyCommandPool(mLogicalDevice, mFrames[i].commandPool, nullptr);

        // Destroy semaphores and fence
        vkDestroySemaphore(mLogicalDevice, mFrames[i].presentSemaphore, nullptr);
        vkDestroyFence(mLogicalDevice, mFrames[i].renderFence, nullptr);

        // Flush deletion queue
        mFrames[i].deleteQueue.Flush();
    }

    // Destroy swap chain semaphores
    for (int i = 0; i < mRenderSemaphoreList.size(); ++i)
    {
        vkDestroySemaphore(mLogicalDevice, mRenderSemaphoreList[i], nullptr);
    }
    mRenderSemaphoreList.clear();

    // Destroy swap chain
    DestroySwapChain();

    // Flush main deletion queue
    mMainDeleteQueue.Flush();

    // Destroy pipelines
    DestroyGraphicsPipeline();

    // Destroy vk logical device during exit
    vkDestroyDevice(mLogicalDevice, nullptr);

    // Destroy vk surface
    vkDestroySurfaceKHR(mInst, mSurface, nullptr);

    // Destroy vk instance during exit
    vkDestroyInstance(mInst, nullptr);
}

void VulkanRenderer::BeginRender(glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm)
{
    // nullptr check (if framebuffer is not bind, don't render)
    if (!mDrawImg || !mDrawExtent)
    {
        return;
    }

    // Get current frame
    FrameData& frame = GetCurrFrame();

    // Wait until previous frame has finished
    // VK_TRUE - Wait for all fences, UINT64_MAX - Disable timeout, make it wait till all fences are done
    vkWaitForFences(mLogicalDevice, 1, &frame.renderFence, VK_TRUE, UINT64_MAX);

    // Clear descriptors at start of frame
    frame.deleteQueue.Flush();
    frame.frameDescriptors.ClearPools(mLogicalDevice);

    // Get image from swap chain
    VkResult res = vkAcquireNextImageKHR(mLogicalDevice, mSwapChain, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &mSwapChainImageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        RecreateSwapChain();
        return;
    }
    else if ((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR)) 
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // Reset fences after wait
    vkResetFences(mLogicalDevice, 1, &frame.renderFence);

    // Reset command buffer first
    VkCommandBuffer& cmd = frame.commandBuffer;
    vkResetCommandBuffer(cmd, 0);

    // Setup command buffer begin info
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Begin command buffer
    if (vkBeginCommandBuffer(cmd, &info) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Clear buffer
    VulkanUtils::TransitionImage(cmd, mDrawImg->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    ClearBuffer();

    // Transition to framebuffer/depthbuffer image
    VulkanUtils::TransitionImage(cmd, mDrawImg->image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VulkanUtils::TransitionImage(cmd, mDepthImg->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    // Allocate new uniform buffer for scene data, add to frame's delete queue
    frame.sceneDataBuffer.CreateBuffer(mAllocator, sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    frame.deleteQueue.AddToList([=, this]() 
    {
        vmaDestroyBuffer(mAllocator, frame.sceneDataBuffer.buffer, frame.sceneDataBuffer.allocation);
    });

    // Reverse depth
    glm::mat4 proj = _projXForm;
    proj[1][1] *= -1;

    // Set view and proj matrices
    sceneData.view = _viewXForm;
    sceneData.proj = proj;

    // Write to buffer
    GPUSceneData* sceneUniformData = (GPUSceneData*)frame.sceneDataBuffer.allocation->GetMappedData();
    *sceneUniformData = sceneData;

    // Create descriptor set to bind and update buffer
    mGlobalDescriptor = frame.frameDescriptors.Allocate(mLogicalDevice, mGpuSceneDataDescLayout, 0);
    VulkanDescriptorWriter writer{};
    writer.WriteBuffer(0, frame.sceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.WriteAccelerationStructure(1, &tlasData.tlasHandle);
    writer.WriteBuffer(2, mLUTResources.indexBuffer.buffer, mLUTResources.indexBufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.WriteBuffer(3, mLUTResources.uvBuffer.buffer, mLUTResources.uvBufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.WriteBuffer(4, mLUTResources.lutBuffer.buffer, mLUTResources.lutBufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.UpdateSet(mLogicalDevice, mGlobalDescriptor);

    // Update ReSTIR resources
    UpdateReSTIRResources();

    // Get colorAttachmentInfo and renderInfo
    VkRenderingAttachmentInfo colorAttachment = VulkanUtils::GetAttachmentInfo(mDrawImg->imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = VulkanUtils::GetDepthAttachmentInfo(mDepthImg->imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    VkRenderingInfo renderInfo = VulkanUtils::GetRenderInfo(*mDrawExtent, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderInfo);
}

void VulkanRenderer::EndRender()
{
    // Get current frame data and command buffer
    FrameData& frame = GetCurrFrame();
    VkCommandBuffer& cmd = frame.commandBuffer;
    VkImage& swapChainImg = mSwapChainImages[mSwapChainImageIndex];

    // End render
    vkCmdEndRendering(cmd);

    // Transition draw and swapchain image
    VulkanUtils::TransitionImage(cmd, mDrawImg->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VulkanUtils::TransitionImage(cmd, swapChainImg, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy framebuffer image onto swapchain image
    VulkanUtils::CopyImageToImage(cmd, mDrawImg->image, swapChainImg, *mDrawExtent, mSwapChainExtent);

    // Transition image back to color attachment optimal to render imgui
    VulkanUtils::TransitionImage(cmd, swapChainImg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Render imgui
    RenderImGUI();

    // Transition swapchain image layout to presentation mode
    VulkanUtils::TransitionImage(cmd, swapChainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // End command buffer
    vkEndCommandBuffer(cmd);

    // Prepare submission to queue
    // Get CBO submit info
    VkCommandBufferSubmitInfo cmdSubmitInfo = VulkanUtils::GetCommandBufferSubmitInfo(cmd);

    // Get semaphore submit info
    VkSemaphoreSubmitInfo swapChainSemaphoreWait = VulkanUtils::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame.presentSemaphore);
    VkSemaphoreSubmitInfo renderSemaphoreSignal = VulkanUtils::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, mRenderSemaphoreList[mSwapChainImageIndex]);

    // Get submit info
    VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdSubmitInfo, &renderSemaphoreSignal, &swapChainSemaphoreWait);

    // Submit command buffer to queue
    VkResult res = vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, frame.renderFence);
    if (res != VK_SUCCESS)
    {
         throw std::runtime_error("Failed to submit command buffer!");
    }

    // Init presentation info
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &mSwapChain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &mRenderSemaphoreList[mSwapChainImageIndex];
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &mSwapChainImageIndex;

    // Submit request to present image to swap chain
    res = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if ((res == VK_ERROR_OUT_OF_DATE_KHR) || (res == VK_SUBOPTIMAL_KHR) || (windowsMgr->mResizeGraphics))
    {
        windowsMgr->mResizeGraphics = false;
        RecreateSwapChain();
    }
    else if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Move to next frame
    mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    // Post render swap ReSTIR buffers
    mReSTIRHandler.PostRenderSwapBuffers();
}

void VulkanRenderer::WaitDeviceIdle(void)
{
    // Wait for logical device to finish operations before exit
    vkDeviceWaitIdle(mLogicalDevice);
}

bool VulkanRenderer::LoadShadersToRenderer(Shader& _shader, std::string const& _shaderName)
{
    // Get spv file paths
    std::string vertShaderFileName = SHADER_DIR + _shaderName + std::string(".vert.spv");
    std::string fragShaderFileName = SHADER_DIR + _shaderName + std::string(".frag.spv");

    // Load vert and frag shader code
    std::vector<uint32_t> vertShaderCode = ReadShaderFile(vertShaderFileName);
    std::vector<uint32_t> fragShaderCode = ReadShaderFile(fragShaderFileName);

    // Create shader modules
    VkShaderModule* vertShader = CreateShaderModule(vertShaderCode);
    VkShaderModule* fragShader = CreateShaderModule(fragShaderCode);

    // Set shader name and add to shader map
    _shader.shaderName = _shaderName;
    mShaderMap.emplace(_shader.shaderName, VulkanShader{});
    mShaderMap[_shaderName].SetShaders(vertShader, fragShader);

    // Reflect shader to load shader bindings
    mShaderMap[_shaderName].ReflectShader(vertShaderCode, mMinUboOffsetAlignment);
    mShaderMap[_shaderName].ReflectShader(fragShaderCode, mMinUboOffsetAlignment);

    // Create descriptor set layout
    mShaderMap[_shaderName].BuildDescriptorSetLayout(mLogicalDevice);

    // Loading complete, return true once done
    return true;
}

void VulkanRenderer::UnloadShader(Shader& _shader)
{
    // Free descriptor set layout
    vkDestroyDescriptorSetLayout(mLogicalDevice, mShaderMap[_shader.shaderName].GetDescriptorSetLayout(), nullptr);

    // Destroy shader module after pipeline is created
    VkShaderModule* vertShader = mShaderMap[_shader.shaderName].GetVertShader();
    VkShaderModule* fragShader = mShaderMap[_shader.shaderName].GetFragShader();
    vkDestroyShaderModule(mLogicalDevice, *vertShader, nullptr);
    vkDestroyShaderModule(mLogicalDevice, *fragShader, nullptr);

    // Free memory
    delete vertShader;
    delete fragShader;
}

void VulkanRenderer::LoadShaderInfo(Shader& _shader)
{
    // Get shader from shadermap
    VulkanShader& shader = mShaderMap[_shader.shaderName];

    // Add shader uniforms to shader
    std::unordered_map<std::string, VulkanShaderBindings> shaderBindings = shader.GetBindings();
    std::unordered_map<std::string, VulkanShaderBindings>::iterator iter = shaderBindings.begin();
    for (iter; iter != shaderBindings.end(); ++iter)
    {
        // Add uniforms
        // Vector2/3/4
        if (iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_VECTOR)
        {
            // Get vector count, match it to respective vector type
            UniformType type = static_cast<UniformType>(iter->second.vectorCount - 2); // Minus two to match enum
            _shader.AddUniform(Uniform{ type, iter->second.name });
        }
        // Float/double
        else if (iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT)
        {
            // Check if it's double/float
            UniformType type = (iter->second.isDouble) ? UniformType::UT_DOUBLE : UniformType::UT_FLOAT;
            _shader.AddUniform(Uniform{ type, iter->second.name });
        }
        // Int/Uint
        else if (iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT)
        {
            // Init type
            UniformType type{};

            // Check if it's a texture index
            if (iter->second.isTextureIndex)
            {
                // Set type as texture index
                type = UniformType::UT_TEXTURE_INDEX;
            }
            else
            {
                // Check for signedness, set type as int/uint
                type = (iter->second.isSigned)? UniformType::UT_INT : UniformType::UT_UNSIGNED_INT;
            }

            // Add int type
            _shader.AddUniform(Uniform{ type, iter->second.name });
        }
        // Image
        else if ((iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE)   ||
                 (iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER) ||
                 (iter->second.dataType & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE))
        {
            // Add sampler2d type
            _shader.AddUniform(Uniform{ UT_SAMPLER2D, iter->second.name });
        }
    }
}

void VulkanRenderer::LoadMaterialToRenderer(Material& _mat)
{
    // Get mat and shader name
    std::string matName = _mat.GetName();
    std::string shaderName = _mat.GetShader();

    // If it already exists, don't add it
    if (mMatMap.find(_mat.GetName()) != mMatMap.end())
        return;

    // Create material pipeline
    CreateGraphicsPipeline(_mat);

    // Add to mat map
    mMatMap.emplace(matName, VulkanMaterialData{});
    VulkanMaterialData& matData = mMatMap[matName];

    // Create UBO for material
    VulkanShader& shader = mShaderMap[shaderName];
    matData.uniformBufferSize = shader.GetUBOSize();
    if (matData.uniformBufferSize > 0)
    {
        // Create buffer, initialize buffer data to 0
        matData.uniformBuffer.CreateBuffer(mAllocator, matData.uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        matData.uniformBufferMappedData = matData.uniformBuffer.info.pMappedData;
        std::memset(matData.uniformBufferMappedData, 0, matData.uniformBufferSize);
    }

    // Allocate descriptor set for material
    VkDescriptorSetLayout matDescSetLayout = shader.GetDescriptorSetLayout();
    matData.descriptorSet = mDescriptorPool.Allocate(mLogicalDevice, matDescSetLayout, 0);
    
    // Bind resources to descriptor set
    matData.writer.Clear();
    std::unordered_map<std::string, VulkanShaderBindings> const& bindings = shader.GetBindings();
    std::unordered_map<std::string, VulkanShaderBindings>::const_iterator iter = bindings.begin();
    for (iter; iter != bindings.end(); ++iter)
    {
        // Skip set 0 & 1
        if (iter->second.setIndex < MATERIAL_MIN_INDEX)
            continue;

        // Uniform buffer
        if (iter->second.type == VulkanShaderBindings::BindingType::BT_UNIFORM_BUFFER)
        {
            matData.writer.WriteBuffer(
                iter->second.bindingIndex,
                matData.uniformBuffer.buffer,
                iter->second.size,
                iter->second.offset,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
        // Sampler2D
        else if (iter->second.type == VulkanShaderBindings::BindingType::BT_COMBINED_IMAGE_SAMPLER)
        {
            // Get texture entry, if it doesn't exist, skip
            std::optional<TextureEntry> texEntry = _mat.GetUniform<TextureEntry>(iter->second.name);
            if (texEntry == std::nullopt)
                continue;

            // Texture exists, bind it to descriptor set
            matData.writer.WriteImage(
                iter->second.bindingIndex,
                mTextureList[texEntry->ID].imageView,
                mDefaultSamplerLinear,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );
        }
    }

    matData.writer.UpdateSet(mLogicalDevice, matData.descriptorSet);
}

void VulkanRenderer::LoadUniformsToShader(Material const& _mat, Shader const& _shader)
{
    // Get shader bindings
    VulkanShader& vkShader = mShaderMap[_shader.shaderName];
    std::unordered_map<std::string, VulkanShaderBindings> const& bindings = vkShader.GetBindings();

    // Get mapped data
    void* mappedData = mMatMap[_mat.GetName()].uniformBufferMappedData;

    // Add uniform variables to map
    std::vector<Uniform> const& shaderUniformList = _shader.GetUniformList();
    for (size_t i = 0; i < shaderUniformList.size(); ++i)
    {
        // Get current uniform
        Uniform const& u = shaderUniformList[i];

        // Get binding offset
        size_t offset = bindings.at(u.uniformName).offset;

        // Set value based on type
        if (u.type == UT_FLOAT_VEC2)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<glm::vec2>(u.uniformName));
        }
        else if (u.type == UT_FLOAT_VEC3)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<glm::vec3>(u.uniformName));
        }
        else if (u.type == UT_FLOAT_VEC4)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<glm::vec4>(u.uniformName));
        }
        else if (u.type == UT_FLOAT)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<float>(u.uniformName));
        }
        else if (u.type == UT_DOUBLE)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<double>(u.uniformName));
        }
        else if (u.type == UT_INT)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<int>(u.uniformName));
        }
        else if (u.type == UT_UNSIGNED_INT)
        {
            SetUniformToShader(mappedData, offset, _mat.GetUniform<unsigned>(u.uniformName));
        }
        else if (u.type == UT_TEXTURE_INDEX)
        {
            std::optional<TextureEntry> texEntry = _mat.GetUniform<TextureEntry>(u.uniformName);
            if (!texEntry.has_value())
            {
                std::cout << "Texture entry not found for uniform: " << u.uniformName << std::endl;
                continue;
            }

            SetUniformToShader(mappedData, offset, static_cast<int>(texEntry.value().ID));
        }
        else if (u.type == UT_SAMPLER2D)
        {
            //// Get texture entry, if it doesn't exist, skip
            //std::optional<TextureEntry> texEntry = _mat.GetUniform<TextureEntry>(iter->second.name);
            //if (texEntry == std::nullopt)
            //    continue;

            //// Texture exists, bind it to descriptor set
            //AllocatedImage* texImg = mTextureList[texEntry->ID];
            //mMatMap[_mat.GetName()].writer.WriteImage(
            //    iter->second.bindingIndex,
            //    texImg->imageView,
            //    mDefaultSamplerLinear,
            //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            //    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            //);
        }
    }
}

void VulkanRenderer::FreeMaterial(std::string const& _matName)
{
    // Destroy UBO
    VulkanMaterialData& matData = mMatMap[_matName];
    if (matData.uniformBufferSize > 0)
    {
        matData.uniformBuffer.DestroyBuffer(mAllocator);
    }

    // Remove from mat map
    mMatMap.erase(_matName);
}

void VulkanRenderer::AddFrameBuffer(std::string const& _fboName)
{
    std::string fboName = _fboName;
    mFrameBuffers.emplace(fboName, VulkanFrameBuffer{});
}

void VulkanRenderer::CreateFrameBuffer(std::string const& _fboName, bool _isFloatingPtFBO)
{
    // Init FBO
    mFrameBuffers[_fboName].isFloatingPtFBO = _isFloatingPtFBO;

    // Create a new image
    AllocatedImage& drawImg = mFrameBuffers[_fboName].drawImg;
    VkExtent2D& extent = mFrameBuffers[_fboName].extent;

    // Set FBO 2D draw image extent
    extent = VkExtent2D{ mSwapChainExtent.width, mSwapChainExtent.height, };

    // Set 3D draw image extent
    VkExtent3D drawImageExtent = { mSwapChainExtent.width, mSwapChainExtent.height, 1 };

    // Set image format (float or int)
    drawImg.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    drawImg.imageExtent = drawImageExtent;

    // Set image usage flags
    VkImageUsageFlags drawImageUsageFlags{};
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Set image create info
    VkImageCreateInfo imgCreateInfo = VulkanUtils::GetImageCreateInfo(drawImg.imageFormat, drawImageUsageFlags, drawImageExtent);

    // Set alloc create info to allocate image memory from gpu local memory
    VmaAllocationCreateInfo imgAllocCreateInfo{};
    imgAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imgAllocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create image
    if (vmaCreateImage(mAllocator, &imgCreateInfo, &imgAllocCreateInfo, &drawImg.image, &drawImg.allocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create FBO's image!");
    }

    // Set image view create info
    VkImageViewCreateInfo imageViewCreateInfo = VulkanUtils::GetImageViewCreateInfo(drawImg.imageFormat, drawImg.image, VK_IMAGE_ASPECT_COLOR_BIT);

    // Create image view
    if (vkCreateImageView(mLogicalDevice, &imageViewCreateInfo, nullptr, &drawImg.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create FBO's image view!");
    }

    // Create depth buffer
    AllocatedImage& depthImg = mFrameBuffers[_fboName].depthImg;
    depthImg.imageFormat = VK_FORMAT_D32_SFLOAT;
    depthImg.imageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageCreateInfo depthImageInfo = VulkanUtils::GetImageCreateInfo(depthImg.imageFormat, depthImageUsages, drawImageExtent);

    // Allocate and create image for depth buffer
    vmaCreateImage(mAllocator, &depthImageInfo, &imgAllocCreateInfo, &depthImg.image, &depthImg.allocation, nullptr);

    // Build image view for depth image to use for rendering
    VkImageViewCreateInfo depthViewInfo = VulkanUtils::GetImageViewCreateInfo(depthImg.imageFormat, depthImg.image, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (vkCreateImageView(mLogicalDevice, &depthViewInfo, nullptr, &depthImg.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create FBO's image view!");
    }

    // Init draw and depth images
    mDrawImg = &mFrameBuffers[_fboName].drawImg;
    mDepthImg = &mFrameBuffers[_fboName].depthImg;
}

void VulkanRenderer::DeleteFrameBuffer(std::string const& _name)
{
    // Destroy draw image
    AllocatedImage& drawImg = mFrameBuffers[_name].drawImg;
    vkDestroyImageView(mLogicalDevice, drawImg.imageView, nullptr);
    vmaDestroyImage(mAllocator, drawImg.image, drawImg.allocation);

    // Destroy depth image
    AllocatedImage& depthImg = mFrameBuffers[_name].depthImg;
    vkDestroyImageView(mLogicalDevice, depthImg.imageView, nullptr);
    vmaDestroyImage(mAllocator, depthImg.image, depthImg.allocation);

    // Remove from map
    mFrameBuffers.erase(_name);
}

void VulkanRenderer::BindFrameBuffer(std::string const& _name)
{
    // Check if name exists in fbo map
    if (mFrameBuffers.find(_name) == mFrameBuffers.end())
        return;

    // Set draw image and extent
    VulkanFrameBuffer& fbo = mFrameBuffers[_name];
    mDrawImg = &fbo.drawImg;
    mDrawExtent = &fbo.extent;
    mDepthImg = &fbo.depthImg;

    // Send framebuffer width/height to GPU
    sceneData.fboDims = { mDrawExtent->width, mDrawExtent->height };
}

void VulkanRenderer::UnbindFrameBuffer(void)
{
    mDrawImg = nullptr;
    mDrawExtent = nullptr;
    mDepthImg = nullptr;
}

Texture& VulkanRenderer::GetFrameBufferTexture(std::string const& _name)
{
    return frameBufferMap[_name].tex;
}

unsigned VulkanRenderer::GetFrameBufferID(std::string const& _name)
{
    return frameBufferMap[_name].fboid;
}

void VulkanRenderer::RescaleFrameBuffer(std::string const& _name, int _width, int _height, bool _lockAspectRatio, bool _isFloatingPtFBO)
{
}

void VulkanRenderer::LoadMeshToRenderer(Model::Mesh& _mesh)
{
    // Get vertex count
    const size_t vtxCount = _mesh.posVtxList.size();
    
    // Build vertex list
    std::vector<Vertex> vertices;
    vertices.resize(vtxCount);
    for (size_t i = 0; i < vtxCount; ++i)
    {
        vertices[i].position = _mesh.posVtxList[i];
        vertices[i].uvX = _mesh.uvVertex[i].x;
        vertices[i].uvY = _mesh.uvVertex[i].y;
        vertices[i].normal = _mesh.normals[i];
        vertices[i].tangent = _mesh.tangents[i];
    }

    // Get vertex and idx buffer sizes
    size_t const vertBufferSize = vertices.size() * sizeof(Vertex);
    size_t const idxBufferSize = _mesh.idxVertex.size() * sizeof(uint32_t);

    // Set indices count
    _mesh.indicesCount = _mesh.idxVertex.size();

    // Create vertex buffer and find address of vertex buffer
    _mesh.meshBuffer.vertexBuffer.CreateBuffer(mAllocator, vertBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
    VkBufferDeviceAddressInfo vertBufferDeviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = _mesh.meshBuffer.vertexBuffer.buffer };
    _mesh.meshBuffer.vertexBufferAddress = vkGetBufferDeviceAddress(mLogicalDevice, &vertBufferDeviceAdressInfo);

    // Create index buffer and find address of index buffer
    _mesh.meshBuffer.indexBuffer.CreateBuffer(mAllocator, idxBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
    VkBufferDeviceAddressInfo idxBufferDeviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = _mesh.meshBuffer.indexBuffer.buffer };
    _mesh.meshBuffer.indexBufferAddress = vkGetBufferDeviceAddress(mLogicalDevice, &idxBufferDeviceAdressInfo);

    // Copy vertex buffer and index buffer
    AllocatedBuffer staging{}; 
    staging.CreateBuffer(mAllocator, vertBufferSize + idxBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data = staging.allocation->GetMappedData();
    memcpy(data, vertices.data(), vertBufferSize);
    memcpy((char*)data + vertBufferSize, _mesh.idxVertex.data(), idxBufferSize);

    // Run GPU side command to perform copy
    ImmediateSubmit([&](VkCommandBuffer cmd) 
    {
        // Copy vertex
        VkBufferCopy vertexCopy{};
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertBufferSize;
        vkCmdCopyBuffer(cmd, staging.buffer, _mesh.meshBuffer.vertexBuffer.buffer, 1, &vertexCopy);

        // Copy indices
        VkBufferCopy indexCopy{};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertBufferSize;
        indexCopy.size = idxBufferSize;
        vkCmdCopyBuffer(cmd, staging.buffer, _mesh.meshBuffer.indexBuffer.buffer, 1, &indexCopy);
    });

    // Destroy staging buffer once done
    staging.DestroyBuffer(mAllocator);

    // Clear lists
    //_mesh.posVtxList.clear();
    //_mesh.idxVertex.clear();
    //_mesh.normals.clear();
    //_mesh.tangents.clear();
    //_mesh.uvVertex.clear();
}

void VulkanRenderer::FreeMesh(Model::Mesh& _mesh)
{
    _mesh.meshBuffer.vertexBuffer.DestroyBuffer(mAllocator);
    _mesh.meshBuffer.indexBuffer.DestroyBuffer(mAllocator);
}

void VulkanRenderer::LoadTextureToRenderer(Texture& _texture)
{
    // Compute data size and create buffer
    size_t dataSize = _texture.width * _texture.height * _texture.numComponents;
    AllocatedBuffer uploadBuffer{};
    uploadBuffer.CreateBuffer(mAllocator, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // Copy data to upload buffer
    if (uploadBuffer.info.pMappedData != nullptr)
        memcpy(uploadBuffer.info.pMappedData, _texture.textureData, dataSize);

    // Create image
    VkExtent3D texSize = VkExtent3D(_texture.width, _texture.height, 1);
    VkFormat texFormat = static_cast<VkFormat>(_texture.format);
    VkImageUsageFlags texUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    bool isMipmapped = (_texture.mipMapCount > 0);
    AllocatedImage newImg{};
    newImg.CreateImage(mLogicalDevice, mAllocator, texSize, texFormat, texUsage, isMipmapped);

    // Immediate submit
    ImmediateSubmit([&](VkCommandBuffer cmd) 
    {
        VulkanUtils::TransitionImage(cmd, newImg.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Set copy region
        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = texSize;

        // Copy the buffer into the image 
        vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImg.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        VulkanUtils::TransitionImage(cmd, newImg.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    // Destroy buffer after submit
    uploadBuffer.DestroyBuffer(mAllocator);

    // Add to list
    mTextureList.emplace_back(newImg);
    _texture.textureID = mTextureList.size() - 1;
}

void VulkanRenderer::FreeTexture(Texture& _texture)
{
    mTextureList[_texture.textureID].DestroyImage(mLogicalDevice, mAllocator);
}

void VulkanRenderer::SetRenderMode(RENDER_MODE _mode)
{
}

void VulkanRenderer::SetCullStatus(bool _enable)
{
}

void VulkanRenderer::SetCullMode(Material::CULL_MODE _mode)
{
}

void VulkanRenderer::ResetCullMode(void)
{
}

uint32_t VulkanRenderer::ReadPixel(std::string const& _fboName, int _x, int _y)
{
    return 0;
}

void VulkanRenderer::SetDepthTestStatus(bool _isEnable)
{
}

void VulkanRenderer::RenderSkySphere(Model::Mesh const& _mesh, Shader& _shader)
{
}

void VulkanRenderer::SetAmbientLight(glm::vec3 const& _color, float _intensity)
{
    // Set ambient light color and intensity in scene data
    sceneData.ambientColorIntensity = glm::vec4(_color, _intensity);
}

void VulkanRenderer::SetPointLight(glm::vec3 const& _worldPos, glm::vec3 const& _intensity, float _range, int _index)
{
    // Set point light position, intensity and range in scene data
    sceneData.pointLights[_index].position = glm::vec4(_worldPos, 1.0f);
    sceneData.pointLights[_index].intensity = glm::vec4(_intensity, _range);
}

void VulkanRenderer::SetDirectionalLight(glm::vec3 const& _dir, glm::vec3 const& _intensity, int _index)
{
    // Set directional light direction and intensity in scene data
    sceneData.dirLights[_index].direction = glm::vec4(_dir, 0.0f);
    sceneData.dirLights[_index].intensity = glm::vec4(_intensity, 0.0f);
}

void VulkanRenderer::SetLightCounts(int _pointLightCount, int _dirLightCount)
{
    // Set light counts in scene data
    sceneData.lightCounts.x = _pointLightCount;
    sceneData.lightCounts.y = _dirLightCount;
}

void VulkanRenderer::SetCameraPosition(glm::vec3 const& _camPos)
{
    sceneData.camPos = _camPos;
}

void VulkanRenderer::InitImGUI()
{
    // Create descriptor pool for ImGui
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    if (vkCreateDescriptorPool(mLogicalDevice, &poolInfo, nullptr, &imguiPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool for ImGUI");
    }

    // Init ImGUI library
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(windowsMgr->mPtrWindow, true);

    // Init ImGui for vulkan
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = mInst;
    initInfo.PhysicalDevice = mPhysicalDevice;
    initInfo.Device = mLogicalDevice;
    initInfo.Queue = mGraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;
    // Init for dynamic rendering
    initInfo.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &mSwapChainImageFormat;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&initInfo);
    ImGui_ImplVulkan_CreateFontsTexture();

    // Add to delete queue
    mMainDeleteQueue.AddToList([=]() 
    {
        vkDestroyDescriptorPool(mLogicalDevice, imguiPool, nullptr);
    });
}

void VulkanRenderer::RenderImGUI()
{
    // Get current frame's command buffer
    FrameData& frame = GetCurrFrame();
    VkCommandBuffer& cmd = frame.commandBuffer;

    // Get color attachment and render info
    VkRenderingAttachmentInfo colorAttachment = VulkanUtils::GetAttachmentInfo(mSwapChainImageViews[mSwapChainImageIndex], nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VulkanUtils::GetRenderInfo(mSwapChainExtent, &colorAttachment, nullptr);

    // Render for ImGUI
    vkCmdBeginRendering(cmd, &renderInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}

void VulkanRenderer::CreateBLAS(std::unordered_map<std::string, Model*> const& _modelMap)
{
    // Iterate thru model map and create BLAS for each mesh
    std::unordered_map<std::string, Model*>::const_iterator modelMap = _modelMap.begin();
    for (modelMap; modelMap != _modelMap.end(); ++modelMap)
    {
        Model* model = modelMap->second;
        blasIDList.emplace(model->modelName, std::vector<uint32_t>{});
        for (int i = 0; i < model->mMeshes.size(); ++i)
        {
            // Initialize blas data
            BLASData blasData{};

            // Create BLAS for mesh
            // Setup geometry data
            Model::Mesh& mesh = model->mMeshes[i];
            VkAccelerationStructureGeometryTrianglesDataKHR meshData
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
                .vertexData = mesh.meshBuffer.vertexBufferAddress,
                .vertexStride = sizeof(Vertex),
                .maxVertex = static_cast<uint32_t>(mesh.posVtxList.size()),
                .indexType = VK_INDEX_TYPE_UINT32,
                .indexData = mesh.meshBuffer.indexBufferAddress,
            };

            // Create BLAS geometry data
            VkAccelerationStructureGeometryDataKHR geometryData{meshData};
            VkAccelerationStructureGeometryKHR blasGeometry
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                .geometry = geometryData,
                .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
            };

            // Create BLAS build info
            VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .geometryCount = 1,
                .pGeometries = &blasGeometry,
            };

            // Query build size info for BLAS
            uint32_t primitiveCount = static_cast<uint32_t>(mesh.idxVertex.size() / 3);
            VkAccelerationStructureBuildSizesInfoKHR blasBuildSize{ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
            mVkLoader.vklGetAccelerationStructureBuildSizesKHR
            (
                mLogicalDevice,
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &buildGeometryInfo,
                &primitiveCount,
                &blasBuildSize
            );

            // Create buffers for BLAS
            blasData.blasBuffer.CreateBuffer(mAllocator, blasBuildSize.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
            blasData.scratchBuffer.CreateBuffer(mAllocator, blasBuildSize.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

            // Set scratch data device address
            VkBufferDeviceAddressInfo scratchBufferAddrInfo
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = blasData.scratchBuffer.buffer,
            };
            buildGeometryInfo.scratchData.deviceAddress = mVkLoader.vklGetBufferDeviceAddressKHR(mLogicalDevice, &scratchBufferAddrInfo);

            // Initialize blas create info
            VkAccelerationStructureCreateInfoKHR blasCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
                .buffer = blasData.blasBuffer.buffer,
                .offset = 0,
                .size = blasBuildSize.accelerationStructureSize,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            };

            // Create BLAS handle
            VkResult res = mVkLoader.vklCreateAccelerationStructureKHR(mLogicalDevice, &blasCreateInfo, nullptr, &blasData.blasHandle);
            if (res != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create BLAS!");
            }

            buildGeometryInfo.dstAccelerationStructure = blasData.blasHandle;

            // Store BLAS handle in list
            blasDataList.emplace_back(blasData);
            blasIDList[model->modelName].push_back(blasDataList.size() - 1);

            // Prepare build range info
            VkAccelerationStructureBuildRangeInfoKHR blasRangeInfo
            {
                .primitiveCount = primitiveCount,
                .primitiveOffset = 0,
                .firstVertex = mesh.idxVertex[0],
                .transformOffset = 0,
            };

            // Build BLAS
            VkAccelerationStructureBuildRangeInfoKHR* blasRangeInfoPtr = &blasRangeInfo;
            ImmediateSubmit([&](VkCommandBuffer cmd)
            {
                mVkLoader.vklCmdBuildAccelerationStructuresKHR
                (
                    cmd,
                    1,
                    &buildGeometryInfo,
                    &blasRangeInfoPtr
                );
            });
        }
    }
}

void VulkanRenderer::CreateTLAS(void)
{
    // Create buffer for instance data
    uint32_t instanceCount = blasInstanceList.size();
    VkDeviceSize instBufferSize = sizeof(blasInstanceList[0]) * instanceCount;
    tlasData.instanceBuffer.CreateBuffer(mAllocator, instBufferSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    
    // Map instance data to buffer
    void* mappedData = tlasData.instanceBuffer.allocation->GetMappedData();
    memcpy(mappedData, blasInstanceList.data(), instBufferSize);

    // Get instance buffer device address
    VkBufferDeviceAddressInfo instanceAddrInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = tlasData.instanceBuffer.buffer,
    };
    VkDeviceAddress instanceAddr = mVkLoader.vklGetBufferDeviceAddressKHR(mLogicalDevice, &instanceAddrInfo);

    // Prepare instance data
    VkAccelerationStructureGeometryInstancesDataKHR instancesData
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
        .arrayOfPointers = VK_FALSE,
        .data = instanceAddr
    };

    VkAccelerationStructureGeometryDataKHR geometryData
    {
        .instances = instancesData
    };

    VkAccelerationStructureGeometryKHR tlasGeometry
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry = geometryData
    };

    // Record build geometry info
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = 1,
        .pGeometries = &tlasGeometry
    };

    // Query TLAS memory requirements for TLAS
    VkAccelerationStructureBuildSizesInfoKHR tlasBuildSizes{ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
    mVkLoader.vklGetAccelerationStructureBuildSizesKHR
    (
        mLogicalDevice,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildGeometryInfo,
        &instanceCount,
        &tlasBuildSizes
    );

    // Create buffer for TLAS and scratch data
    tlasData.scratchBuffer.CreateBuffer(mAllocator, tlasBuildSizes.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    tlasData.tlasBuffer.CreateBuffer(mAllocator, tlasBuildSizes.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);

    // Get scratch buffer device address
    VkBufferDeviceAddressInfo scratchBufferAddrInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = tlasData.scratchBuffer.buffer,
    };
    buildGeometryInfo.scratchData.deviceAddress = mVkLoader.vklGetBufferDeviceAddressKHR(mLogicalDevice, &scratchBufferAddrInfo);

    // Init TLAS create info
    VkAccelerationStructureCreateInfoKHR tlasCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = tlasData.tlasBuffer.buffer,
        .offset = 0,
        .size = tlasBuildSizes.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
    };

    // Create and save TLAS handle
    mVkLoader.vklCreateAccelerationStructureKHR(mLogicalDevice, &tlasCreateInfo, nullptr, &tlasData.tlasHandle);
    buildGeometryInfo.dstAccelerationStructure = tlasData.tlasHandle;

    // Prepare build range for TLAS
    VkAccelerationStructureBuildRangeInfoKHR tlasRangeInfo
    {
        .primitiveCount = instanceCount,
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0,
    };

    // Build the TLAS
    VkAccelerationStructureBuildRangeInfoKHR* tlasRangeInfoPtr = &tlasRangeInfo;
    ImmediateSubmit([&](VkCommandBuffer cmd)
    {
        mVkLoader.vklCmdBuildAccelerationStructuresKHR
        (
            cmd,
            1,
            &buildGeometryInfo,
            &tlasRangeInfoPtr
        );
    });
}

void VulkanRenderer::CreateBLASInstances(uint32_t _entityID, Model const& _model, glm::mat4 const& _xform)
{
    // Prepare BLAS instances for all meshes in model
    for (int i = 0; i < _model.mMeshes.size(); ++i)
    {
        // Prepare instance data for TLAS
        VkAccelerationStructureDeviceAddressInfoKHR addrInfo
        {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
            .accelerationStructure = blasDataList[blasIDList[_model.modelName][i]].blasHandle,
        };
        VkDeviceAddress blasDeviceAddr = mVkLoader.vklGetAccelerationStructureDeviceAddressKHR(mLogicalDevice, &addrInfo);

        // Convert mat4 (column major) to vkTransformMatrixKHR (row major)
        auto toTransformMatrixKHR = [](const glm::mat4& m) 
        {
            VkTransformMatrixKHR t;
            memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
            return t;
        };

        // Set instance data (tm is identity matrix)
        VkAccelerationStructureInstanceKHR instance
        {
            .transform = toTransformMatrixKHR(_xform * _model.mMeshes[i].initialXFormMat),
            .instanceCustomIndex = _entityID,
            .mask = 0xFF,
            .accelerationStructureReference = blasDeviceAddr
        };
        blasInstanceList.emplace_back(instance);

        // Add to instance ID map
        tlasData.instanceIDMap.try_emplace(_entityID);
        tlasData.instanceIDMap[_entityID].emplace(i, static_cast<int>(blasInstanceList.size()) - 1);
    }
}

void VulkanRenderer::UpdateBLASInstanceTransform(uint32_t _entityID, int _meshIdx, glm::mat4 const& _xform)
{
    // Get instance index from instance ID map
    uint32_t instanceIdx = tlasData.instanceIDMap[_entityID][_meshIdx];

    // Update transform matrix
    auto toTransformMatrixKHR = [](const glm::mat4& m)
    {
        VkTransformMatrixKHR t;
        memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
        return t;
    };
    blasInstanceList[instanceIdx].transform = toTransformMatrixKHR(_xform);
}

void VulkanRenderer::UpdateTLAS(void)
{
    // Update instance buffer with new instance data
    uint32_t instanceCount = blasInstanceList.size();
    VkDeviceSize instBufferSize = sizeof(blasInstanceList[0]) * instanceCount;
    void* mappedData = tlasData.instanceBuffer.allocation->GetMappedData();
    memcpy(mappedData, blasInstanceList.data(), instBufferSize);

    // Get instance buffer device address
    VkBufferDeviceAddressInfo instanceAddrInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = tlasData.instanceBuffer.buffer,
    };
    VkDeviceAddress instanceAddr = mVkLoader.vklGetBufferDeviceAddressKHR(mLogicalDevice, &instanceAddrInfo);

    // Prepare instance data
    VkAccelerationStructureGeometryInstancesDataKHR instancesData
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
        .arrayOfPointers = VK_FALSE,
        .data = instanceAddr
    };

    VkAccelerationStructureGeometryDataKHR geometryData
    {
        .instances = instancesData
    };

    VkAccelerationStructureGeometryKHR tlasGeometry
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry = geometryData
    };

    // Record build geometry info
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo
    {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,
        .srcAccelerationStructure = tlasData.tlasHandle,
        .dstAccelerationStructure = tlasData.tlasHandle,
        .geometryCount = 1,
        .pGeometries = &tlasGeometry,
    };

    // Get scratch buffer device address
    VkBufferDeviceAddressInfo scratchBufferAddrInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = tlasData.scratchBuffer.buffer,
    };
    buildGeometryInfo.scratchData.deviceAddress = mVkLoader.vklGetBufferDeviceAddressKHR(mLogicalDevice, &scratchBufferAddrInfo);

    // Prepare build range for TLAS
    VkAccelerationStructureBuildRangeInfoKHR tlasRangeInfo
    {
        .primitiveCount = instanceCount,
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0,
    };

    // Rebuild the TLAS
    VkAccelerationStructureBuildRangeInfoKHR* tlasRangeInfoPtr = &tlasRangeInfo;
    ImmediateSubmit([&](VkCommandBuffer cmd)
    {
        // Prebuild barrier
        VkMemoryBarrier preBarrier
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
        };
        vkCmdPipelineBarrier
        (
            cmd,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            0,
            1,
            &preBarrier,
            0,
            nullptr,
            0,
            nullptr
        );

        // Rebuild TLAS
        mVkLoader.vklCmdBuildAccelerationStructuresKHR
        (
            cmd,
            1,
            &buildGeometryInfo,
            &tlasRangeInfoPtr
        );

        // Post build barrier
        VkMemoryBarrier postBarrier
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
            .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT
        };
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            1, 
            &postBarrier,
            0, 
            NULL,
            0, 
            NULL 
        );
    });
}

void VulkanRenderer::DestroyRaytracingResources(void)
{
    // Wait for device to be idle
    WaitDeviceIdle();

    // Clear TLAS
    if (tlasData.tlasHandle != VK_NULL_HANDLE)
    {
        mVkLoader.vklDestroyAccelerationStructureKHR(mLogicalDevice, tlasData.tlasHandle, nullptr);
        tlasData.tlasHandle = VK_NULL_HANDLE;
    }

    // Destroy TLAS buffers
    tlasData.tlasBuffer.DestroyBuffer(mAllocator);
    tlasData.scratchBuffer.DestroyBuffer(mAllocator);
    tlasData.instanceBuffer.DestroyBuffer(mAllocator);

    // Clear BLASs and destroy buffers
    for (BLASData& blas : blasDataList)
    {
        if (blas.blasHandle != VK_NULL_HANDLE)
        {
            mVkLoader.vklDestroyAccelerationStructureKHR(mLogicalDevice, blas.blasHandle, nullptr);
            blas.blasHandle = VK_NULL_HANDLE;
        }

        blas.blasBuffer.DestroyBuffer(mAllocator);
        blas.scratchBuffer.DestroyBuffer(mAllocator);
    }

    // Clear containers
    blasDataList.clear();
    blasInstanceList.clear();
    blasIDList.clear();
}

void VulkanRenderer::AddInstanceToLUTList(int _texID, Model const& _model, uint32_t _meshIdx)
{
    // Add instance to LUT list
    LUTInstance newInstance;
    newInstance.textureID = _texID;
    lutList.push_back(newInstance);

    // If model name does not exist, create new entry
    if (lutListCreateHelper.find(_model.modelName) == lutListCreateHelper.end())
    {
        lutListCreateHelper[_model.modelName].resize(_model.mMeshes.size());
    }

    // Add to helper map for LUT list creation
    lutListCreateHelper[_model.modelName][_meshIdx].emplace_back(static_cast<uint32_t>(lutList.size()) - 1);
}

void VulkanRenderer::CreateLUTResources(std::unordered_map<std::string, Model*> const& _modelMap)
{
    // Pack all indices and uvs together, set indices size
    std::vector<uint32_t> indices;
    std::vector<glm::vec2> uvs;
    std::unordered_map<std::string, Model*>::const_iterator iter = _modelMap.begin();
    for (iter; iter != _modelMap.end(); ++iter)
    {
        Model* model = iter->second;
        for (size_t meshIdx = 0; meshIdx < model->mMeshes.size(); ++meshIdx)
        {
            if (lutListCreateHelper.find(model->modelName) != lutListCreateHelper.end())
            {
                // If model has LUT instances, set LUT instance indices and uv offsets
                std::vector<uint32_t>& instanceList = lutListCreateHelper[model->modelName][meshIdx];
                for (size_t instanceIdx = 0; instanceIdx < instanceList.size(); ++instanceIdx)
                {
                    // Set LUT instance indices and uv offsets
                    LUTInstance& instance = lutList[instanceList[instanceIdx]];
                    instance.indexBufferOffset = indices.size();
                    instance.uvBufferOffset = uvs.size();
                }
            }

            // Add indices and uvs to list
            Model::Mesh const& mesh = model->mMeshes[meshIdx];
            indices.insert(indices.end(), mesh.idxVertex.begin(), mesh.idxVertex.end());
            uvs.insert(uvs.end(), mesh.uvVertex.begin(), mesh.uvVertex.end());
        }
    }

    // Set buffer sizes
    mLUTResources.indexBufferSize = indices.size() * sizeof(uint32_t);
    mLUTResources.uvBufferSize = uvs.size() * sizeof(glm::vec2);
    mLUTResources.lutBufferSize = lutList.size() * sizeof(LUTInstance);

    // Create indices buffer and map indices data
    mLUTResources.indexBuffer.CreateBuffer(mAllocator, mLUTResources.indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    void* idxMappedData = mLUTResources.indexBuffer.allocation->GetMappedData();
    memcpy(idxMappedData, indices.data(), mLUTResources.indexBufferSize);

    // Create UV buffer
    mLUTResources.uvBuffer.CreateBuffer(mAllocator, mLUTResources.uvBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    void* uvMappedData = mLUTResources.uvBuffer.allocation->GetMappedData();
    memcpy(uvMappedData, uvs.data(), mLUTResources.uvBufferSize);

    // Create buffer for LUT table
    mLUTResources.lutBuffer.CreateBuffer(mAllocator, mLUTResources.lutBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    void* lutMappedData = mLUTResources.lutBuffer.allocation->GetMappedData();
    memcpy(lutMappedData, lutList.data(), mLUTResources.lutBufferSize);
}

void VulkanRenderer::DestroyLUTResources(void)
{
    // Destroy LUT buffers
    mLUTResources.indexBuffer.DestroyBuffer(mAllocator);
    mLUTResources.uvBuffer.DestroyBuffer(mAllocator);
    mLUTResources.lutBuffer.DestroyBuffer(mAllocator);
}

void VulkanRenderer::CreateReSTIRResources(void)
{
    // Create and bind buffers
    glm::uvec2 renderDims = { mSwapChainExtent.width, mSwapChainExtent.height };
    mReSTIRHandler.CreateReSTIRBuffers(mLogicalDevice, mAllocator, renderDims);
    mReSTIRHandler.BindReSTIRBuffers(mLogicalDevice);

    // Allocate descriptor
    mReSTIRHandler.mReSTIRDescriptor = mDescriptorPool.Allocate(mLogicalDevice, mReSTIRHandler.mReSTIRDescriptorSetLayout, 0);
}

void VulkanRenderer::DestroyReSTIRResources(void)
{
    mReSTIRHandler.DestroyReSTIRResources(mLogicalDevice, mAllocator);
}

void VulkanRenderer::UpdateReSTIRResources(void)
{
    // Update buffers and descriptor set
    VulkanDescriptorWriter writer;
    writer.WriteBuffer(0, mReSTIRHandler.mCurrReservoirBuffer.buffer, mReSTIRHandler.mReservoirBufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.WriteBuffer(1, mReSTIRHandler.mPrevReservoirBuffer.buffer, mReSTIRHandler.mReservoirBufferSize, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.WriteImage(2, mReSTIRHandler.mCurrNormalsBuffer.imageView, mDefaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.WriteImage(3, mReSTIRHandler.mPrevNormalsBuffer.imageView, mDefaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.WriteImage(4, mReSTIRHandler.mCurrVelocityBuffer.imageView, mDefaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.WriteImage(5, mDepthImg->imageView, mDefaultSamplerLinear, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(mLogicalDevice, mReSTIRHandler.mReSTIRDescriptor);
}

VulkanPipeline& VulkanRenderer::CreatePipeline(Material const& _mat)
{
    // Init vertex shader pipeline stage info
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkShaderModule* vertShader = mShaderMap[_mat.GetShader()].GetVertShader();
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = *vertShader;
    vertShaderStageInfo.pName = "main";

    // Init frag shader pipeline stage info
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    VkShaderModule* fragShader = mShaderMap[_mat.GetShader()].GetFragShader();
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = *fragShader;
    fragShaderStageInfo.pName = "main";

    // Specify viewport info (similar to glViewport)
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(mSwapChainExtent.width);
    viewport.height = static_cast<float>(mSwapChainExtent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    // Specify scissor info
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapChainExtent;

    // Create new Vulkan pipeline object
    mPipelineMap.emplace(_mat.GetName(), VulkanPipeline{});
    VulkanPipeline& pipeline = mPipelineMap[_mat.GetName()];

    // Set all required create infos and shader stages
    pipeline.SetShaderStages(vertShaderStageInfo, fragShaderStageInfo);

    // Return constructed pipeline
    return pipeline;
}

void VulkanRenderer::InitDefaultSamplers(void)
{
    // Init linear sampler
    VkSamplerCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkCreateSampler(mLogicalDevice, &createInfo, nullptr, &mDefaultSamplerLinear);

    // Add to delete queue
    mMainDeleteQueue.AddToList([&]() {
        vkDestroySampler(mLogicalDevice, mDefaultSamplerLinear, nullptr);
    });
}

void VulkanRenderer::CreateInstance(void)
{
    // If validation layers are enabled, check if requested layers exist
    if (mEnableValidationLayers && !CheckValidationLayerSupport())
    {
        std::cout << "Requested validation layer does not exist!" << std::endl;
        std::cout << "Validation layers are disabled." << std::endl;
        mEnableValidationLayers = false;
    }

    // Init appinfo
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_4;
    appInfo.pEngineName = "SKRenderer";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = windowsMgr->mTitle.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    // Init instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Specify desired global extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // Enable validation layers if required
    if (mEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
        createInfo.ppEnabledLayerNames = mValidationLayers.data();
    }
    else
    {
        // No validation layers
        createInfo.enabledLayerCount = 0;
    }

    // Create instance
    if (vkCreateInstance(&createInfo, nullptr, &mInst) != VK_SUCCESS)
    {
        // Failed to create instance, exit program
        throw std::runtime_error("Failed to create instance!");
    }
}

bool VulkanRenderer::CheckValidationLayerSupport(void)
{
    // Fetch available layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    // Check if all layers exists in available layers
    for (const char* layerName : mValidationLayers)
    {
        bool layerFound{};
        for (const auto& layerProps : availableLayers)
        {
            if (strcmp(layerName, layerProps.layerName) == 0)
            {
                // Requested layer is found
                layerFound = true;
                break;
            }
        }

        // If layer does not exist, return false
        if (!layerFound) 
            return false;
    }
    
    // All requested layers are available
    return true;
}

void VulkanRenderer::CreateSurface(void)
{
    // Create surface
    GLFWwindow* window = windowsMgr->mPtrWindow;
    VkResult result = glfwCreateWindowSurface(mInst, window, nullptr, &mSurface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void VulkanRenderer::PickPhysicalDevice(void)
{
    // Get devices count
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(mInst, &deviceCount, nullptr);

    // If devices count is 0, no vulkan-supporting GPUs exists
    if (deviceCount == 0) 
    {
        throw std::runtime_error("No vulkan-supporting GPUs exists");
    }

    // Store all physical device handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInst, &deviceCount, devices.data());

    // Pick and set suitable device
    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto & device: devices)
    {
        int score = RateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Set most suitable device
    if (candidates.rbegin()->first > 0)
    {
        mPhysicalDevice = candidates.rbegin()->second;
    }
    else
    {
        // If all devices are unsuitable, exit
        throw std::runtime_error("No suitable device is found");
    }

    // Set min UBO offset alignment
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);
    mMinUboOffsetAlignment = props.limits.minUniformBufferOffsetAlignment;
}

int VulkanRenderer::RateDeviceSuitability(VkPhysicalDevice const& _device)
{
    // Get device properties and features
    VkPhysicalDeviceProperties deviceProps;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(_device, &deviceProps);
    vkGetPhysicalDeviceFeatures(_device, &deviceFeatures);

    // For descriptor indexing features
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT, nullptr };
    VkPhysicalDeviceFeatures2 deviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };
    vkGetPhysicalDeviceFeatures2(_device, &deviceFeatures2);

    // If device does not support basic shaders, don't use it
    if (!deviceFeatures.geometryShader)
    {
        return 0;
    }

    // If required queue families does not exist, don't use it
    QueueFamilyIndices indices = FindQueueFamilies(_device);
    if (!indices.IsComplete())
    {
        return 0;
    }

    // If device doesn't support required extensions, don't use it
    if (!CheckDeviceExtensionSupport(_device))
    {
        return 0;
    }

    // If swap chain support is insufficient, don't use it
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_device);
    bool isSupportSufficient = (!swapChainSupport.formats.empty()) && (!swapChainSupport.presentModes.empty());
    if (!isSupportSufficient)
    {
        return 0;
    }
    
    // If bindless features are not supported, don't use it
    bool supportBindless = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;
    if (!supportBindless)
    {
        return 0;
    }

    // Prefer dedicated GPUs
    int score = 0;
    if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Prefer GPU with the highest supported image dimensions
    score += deviceProps.limits.maxImageDimension2D;

    // Return score of GPU
    return score;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice const& _device)
{
    QueueFamilyIndices indices;

    // Retrive list of queue families
    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

    // Find queue family that supports required bits
    int i{};
    for (const auto& queueFamily : queueFamilies) 
    {
        // Check for graphics family
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            indices.graphicsFamily = i;
        }

        // Check for present family
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, mSurface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        // If all queue families are present, exit loop
        if (indices.IsComplete()) 
        {
            break;
        }

        i++;
    }

    // Return indices once done
    return indices;
}

void VulkanRenderer::CreateLogicalDevice(void)
{
    // For chaining features
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Enable raytracing features
    VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeature{};
    asFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    asFeature.accelerationStructure = VK_TRUE;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtpFeature{};
    rtpFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rtpFeature.rayTracingPipeline = VK_TRUE;
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
    rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    rayQueryFeatures.rayQuery = VK_TRUE;

    // Enable vk1.3 features (dynamic rendering and synchro2)
    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    // Enable vk1.2 features (BDA and descriptor indexing)
    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;

    // Enable vk1.1 feeatures
    VkPhysicalDeviceVulkan12Features features11{};
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

    // Form pNext chain
    deviceFeatures2.pNext = &features13;
    features13.pNext = &features12;
    features12.pNext = &features11;
    features11.pNext = &rtpFeature;
    rtpFeature.pNext = &asFeature;
    asFeature.pNext = &rayQueryFeatures;
    rayQueryFeatures.pNext = nullptr;

    // Create logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures2;

    // Get queue family indices
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

    // Setup for adding queue families
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    // Init queue create info for all queue families
    float queuePriority = 1.f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    // Enable device extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

    // Enable validation layers if required
    if (mEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
        createInfo.ppEnabledLayerNames = mValidationLayers.data();
    }
    else
    {
        // No validation layers
        createInfo.enabledLayerCount = 0;
    }

    // Create device
    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS)
    {
        // Failed to create logical device, exit program
        throw std::runtime_error("Failed to create logical device!");
    }

    // Set queues
    vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice const& _device)
{
    // Fetch available extensions
    uint32_t extensionCount{};
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

    // Remove all available extensions
    for (const auto& ext : availableExtensions)
    {
        requiredExtensions.erase(ext.extensionName);
    }

    // If all required extensions are available, list will be empty
    return requiredExtensions.empty();
}

VulkanRenderer::SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice const& _device)
{
    SwapChainSupportDetails details;

    // Query capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, mSurface, &details.capabilities);

    // Query formats
    uint32_t formatCount{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device, mSurface, &formatCount, nullptr);
    if (formatCount > 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, mSurface, &formatCount, details.formats.data());
    }

    // Query presentation modes
    uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device, mSurface, &presentModeCount, nullptr);
    if (formatCount > 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, mSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& _formats)
{
    VkSurfaceFormatKHR defaultFormat{};
    for (const auto& availableFormat : _formats) 
    {
        // Check for preferred floating point based format
        if (availableFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
           (availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT ||
            availableFormat.colorSpace == VK_COLOR_SPACE_BT709_LINEAR_EXT))
        {
            // Return preferred format
            return availableFormat;
        }

        // Check for suitable default format
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            // Return a default integer format
            defaultFormat = availableFormat;
        }
    }

    return defaultFormat;
}

VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& _presentModes)
{
    for (const auto& availablePresentMode : _presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            // If triple buffering is available, use triple buffering
            return availablePresentMode;
        }
    }

    // Use double buffering
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(VkSurfaceCapabilitiesKHR const& _capabilities)
{
    if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return _capabilities.currentExtent;
    }
    else 
    {
        // Get windows's drawable width and height 
        int width = windowsMgr->mDrawableWidth;
        int height = windowsMgr->mDrawableHeight;

        // Cast to uint32_t
        VkExtent2D actualExtent = 
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Bound extents by min/max extent with and height
        actualExtent.width = std::clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

void VulkanRenderer::CreateSwapChain(void)
{
    // Query swap chain support
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);

    // Get format, presentation mode and extents
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    // Decide number of images in swap chain
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // Ensure it doesn't exceed max number of images
    if ((swapChainSupport.capabilities.maxImageCount) > 0 && 
        (imageCount > swapChainSupport.capabilities.maxImageCount))
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Set createinfo for swap chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // Amount of layers each image consits of
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Set queue family info to createInfo
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // No pre-transform
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    // No composite alpha
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Set present mode and enable clipping
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // Set old swap chain
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create swap chain
    if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Retrieve swap chain images
    vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    // Store format and extent
    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
}

void VulkanRenderer::DestroySwapChain(void)
{
    // Destroy framebuffers
    for (auto fbo : mFrameBuffers)
    {
        DeleteFrameBuffer(fbo.first);
    }

    // Destroy vk image views
    for (auto imageView : mSwapChainImageViews)
    {
        vkDestroyImageView(mLogicalDevice, imageView, nullptr);
    }
    mSwapChainImageViews.clear();

    // Destroy vk swap chain
    vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
}

void VulkanRenderer::RecreateSwapChain(void)
{
    // Wait for device to be idle
    vkDeviceWaitIdle(mLogicalDevice);

    // Cleanup current swap chain
    DestroySwapChain();

    // Build new swap chain
    CreateSwapChain();
    CreateImageViews();

    // Build new framebuffers
    for (auto fbo : mFrameBuffers)
    {
        // Create new framebuffer, no need to emplace to map again
        CreateFrameBuffer(fbo.first, fbo.second.isFloatingPtFBO);
    }
}

void VulkanRenderer::CreateImageViews(void)
{
    // Resize image view list
    mSwapChainImageViews.resize(mSwapChainImages.size());

    // Create image views
    for (size_t i = 0; i < mSwapChainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Describes image's purpose and which part of image should be accessed
        // No mipmapping or multiple layers
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Create image view
        VkResult result = vkCreateImageView(mLogicalDevice, &createInfo, nullptr, &mSwapChainImageViews[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void VulkanRenderer::CreateGraphicsPipeline(Material const& _mat)
{
    // Create vkpipeline
    VulkanPipeline& pipeline = CreatePipeline(_mat);

    // Get descriptor set layout
    VulkanShader& shader = mShaderMap[_mat.GetShader()];
    VkDescriptorSetLayout matDescSetLayout = shader.GetDescriptorSetLayout();

    // Get buffer range
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Set layout array
    VkDescriptorSetLayout layouts[] =
    {
        mGpuSceneDataDescLayout,
        mBindlessTextureDescLayout,
        mReSTIRHandler.mReSTIRDescriptorSetLayout,
        matDescSetLayout
    };

    // Init pipeline layout create info
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanUtils::GetPipelineLayoutCreateInfo();
    pipelineLayoutCreateInfo.pSetLayouts = layouts;
    pipelineLayoutCreateInfo.setLayoutCount = 4;
    pipelineLayoutCreateInfo.pPushConstantRanges = &bufferRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    // Create pipeline layout
    if (vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipeline.GetPipelineLayout()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    // Set pipeline properties
    pipeline.SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline.SetPolygonMode(VK_POLYGON_MODE_FILL);
    pipeline.SetCullMode(_mat.GetCullMode());
    pipeline.DisableMultisampling();
    pipeline.EnableAlphaBlending();
    pipeline.EnableDepthTest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
    pipeline.SetColorAttachmentFormat(mDrawImg->imageFormat);
    pipeline.SetDepthFormat(mDepthImg->imageFormat);

    // Build pipeline
    pipeline.BuildPipeline(mLogicalDevice, mDynamicStates);
}

void VulkanRenderer::DestroyGraphicsPipeline(void)
{
    std::unordered_map <std::string, VulkanPipeline>::iterator iter;
    for (iter = mPipelineMap.begin(); iter != mPipelineMap.end(); ++iter)
    {
        // Delete pipelines
        iter->second.Destroy(mLogicalDevice);
    }
    mPipelineMap.clear();
}

std::vector<uint32_t> VulkanRenderer::ReadShaderFile(std::string const& _fileName)
{
    // Open spv shader file
    std::ifstream ifs(_fileName, std::ios::ate | std::ios::binary);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Failed to open shader file!");
    }

    // Load data from shader
    size_t fileSize = static_cast<size_t>(ifs.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    ifs.seekg(0);
    ifs.read((char*)buffer.data(), fileSize);

    // Close file once done
    ifs.close();

    // Return loaded data
    return buffer;
}

VkShaderModule* VulkanRenderer::CreateShaderModule(std::vector<uint32_t> const& _shaderCode)
{
    // Init create info
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = _shaderCode.size() * sizeof(uint32_t);
    createInfo.pCode = _shaderCode.data();

    // Create shader module
    VkShaderModule* shaderModule = new VkShaderModule{};
    if (vkCreateShaderModule(mLogicalDevice, &createInfo, nullptr, shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    // Return shader module once done
    return shaderModule;
}

void VulkanRenderer::CreateCommandPoolAndBuffer(void)
{
    // Init create info
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.pNext = nullptr;

    // Create command pool and command buffers
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        // Create command pool
        if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mFrames[i].commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool!");
        }

        // Allocate command buffer from command pool
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mFrames[i].commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.pNext = nullptr;
        if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &mFrames[i].commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    // Create command pool and buffer for immediate submit
    // Create command pool
    if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mImmediateCmdPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }

    // Create command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mImmediateCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.pNext = nullptr;
    if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &mImmediateCmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    // Add to deletion queue
    mMainDeleteQueue.AddToList([=]() 
    { 
        vkDestroyCommandPool(mLogicalDevice, mImmediateCmdPool, nullptr);
    });
}

void VulkanRenderer::CreateSyncObjects(void)
{
    // Init semaphore create info
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Init fence create info
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // Create fences and semaphores for rendering and presenting for each frame
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        // Create semaphore and fence
        if (vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mFrames[i].presentSemaphore) != VK_SUCCESS ||
            vkCreateFence(mLogicalDevice, &fenceInfo, nullptr, &mFrames[i].renderFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphores!");
        }
    }

    // Create semaphores for fetching swap chain images
    mRenderSemaphoreList.resize(mSwapChainImages.size());
    for (int i = 0; i < mRenderSemaphoreList.size(); ++i)
    {
        if (vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mRenderSemaphoreList[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphore!");
        }
    }

    // Create fence for immediate submit
    if (vkCreateFence(mLogicalDevice, &fenceInfo, nullptr, &mImmediateFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence for immediate submit!");
    }

    // Add to delete queue
    mMainDeleteQueue.AddToList([=]() 
    { 
        vkDestroyFence(mLogicalDevice, mImmediateFence, nullptr); 
    });
}

VulkanRenderer::FrameData& VulkanRenderer::GetCurrFrame(void)
{
    return mFrames[mFrameIndex];
}

void VulkanRenderer::InitMemoryAllocator(void)
{
    // Initialize memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = mPhysicalDevice;
    allocatorInfo.device = mLogicalDevice;
    allocatorInfo.instance = mInst;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &mAllocator);

    // Add dealloc function to list
    mMainDeleteQueue.AddToList([&]() { vmaDestroyAllocator(mAllocator); });
}

void VulkanRenderer::InitFrameBuffers(void)
{
    for (auto fbo : mFrameBuffers)
    {
        // Create vulkan framebuffer
        CreateFrameBuffer(fbo.first, fbo.second.isFloatingPtFBO);
    }
}

void VulkanRenderer::InitDescriptorPools(void)
{
    // Create global descriptor pool
    std::vector<VulkanDescriptorPool::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 },
    };
    mDescriptorPool.InitPool(mLogicalDevice, 10, sizes);

    // For cleaning up of descriptor pool
    mMainDeleteQueue.AddToList([&]()
    {
        mDescriptorPool.DestroyPool(mLogicalDevice);
    });

    // Init frame descriptor pools
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Create a descriptor pool
        std::vector<VulkanDescriptorPool::PoolSizeRatio> frameSizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
            { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 4 }
        };

        mFrames[i].frameDescriptors = VulkanDescriptorPool{};
        mFrames[i].frameDescriptors.InitPool(mLogicalDevice, 1000, frameSizes);

        mMainDeleteQueue.AddToList([&, i]()
        {
            mFrames[i].frameDescriptors.DestroyPool(mLogicalDevice);
        });
    }
}

void VulkanRenderer::InitDescriptors(void)
{
    // Build descriptor layout
    VulkanDescriptor vkDesc{};
    vkDesc.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    mDrawImageDescriptorLayout = vkDesc.Build(mLogicalDevice, VK_SHADER_STAGE_COMPUTE_BIT);

    // Allocate descriptor set for draw image
    mDrawImageDescriptors = mDescriptorPool.Allocate(mLogicalDevice, mDrawImageDescriptorLayout, 0);

    // Write image and update set
    VulkanDescriptorWriter writer;
    writer.WriteImage(0, mDrawImg->imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(mLogicalDevice, mDrawImageDescriptors);

    // For cleaning up of descriptors
    mMainDeleteQueue.AddToList([&]() 
    {
        vkDestroyDescriptorSetLayout(mLogicalDevice, mDrawImageDescriptorLayout, nullptr);
    });

    // Init descriptor for scene data
    VulkanDescriptor desc{};
    desc.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    desc.AddBinding(1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
    desc.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // Index buffer
    desc.AddBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // UV buffer
    desc.AddBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); // LUT buffer
    mGpuSceneDataDescLayout = desc.Build(mLogicalDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    // For cleaning up scene data descriptor layout
    mMainDeleteQueue.AddToList([&]()
    {
        vkDestroyDescriptorSetLayout(mLogicalDevice, mGpuSceneDataDescLayout, nullptr);
    });

    // Init descriptor for bindless textures
    VulkanDescriptor bindlessDesc{};
    bindlessDesc.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, mTextureList.size());
    
    // Set bindless extended info
    VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
    extendedInfo.bindingCount = 1;
    extendedInfo.pBindingFlags = &bindlessFlags;
    
    // Build bindless desc layout
    mBindlessTextureDescLayout = bindlessDesc.Build(mLogicalDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &extendedInfo, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT);

    // For cleaning up bindless textures descriptor layout
    mMainDeleteQueue.AddToList([&]()
    {
        vkDestroyDescriptorSetLayout(mLogicalDevice, mBindlessTextureDescLayout, nullptr);
    });

    // Create descriptor set to bind and update bindless textures
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countAllocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
    uint32_t maxBinding = mTextureList.size();
    countAllocInfo.descriptorSetCount = 1;
    countAllocInfo.pDescriptorCounts = &maxBinding;
    mBindlessTextureDescriptor = mDescriptorPool.Allocate(mLogicalDevice, mBindlessTextureDescLayout, &countAllocInfo);

    // Write bindless texture desc set
    VulkanDescriptorWriter bindlessDescWriter;
    for (int i = 0; i < mTextureList.size(); ++i)
    {
        bindlessDescWriter.WriteImage(0, mTextureList[i].imageView, mDefaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
    }

    // Update set after writing
    bindlessDescWriter.UpdateSet(mLogicalDevice, mBindlessTextureDescriptor);
}

void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer const& _cmd)>&& _func)
{
    // Reset fence
    VkResult res = vkResetFences(mLogicalDevice, 1, &mImmediateFence);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset immediate fence!");
    }
    
    // Reset command buffer
    res = vkResetCommandPool(mLogicalDevice, mImmediateCmdPool, 0);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset immediate command buffers!");
    }

    // Begin command buffer
    VkCommandBuffer cmd = mImmediateCmdBuffer;
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin command buffer!");
    }

    // Record cmd
    _func(cmd);

    // End command buffer
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to end command buffer!");
    }

    // Submit command
    VkCommandBufferSubmitInfo cmdSubmitInfo = VulkanUtils::GetCommandBufferSubmitInfo(cmd);
    VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdSubmitInfo, nullptr, nullptr);
    res = vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, mImmediateFence);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer!");
    }

    // Wait for fence
    res = vkWaitForFences(mLogicalDevice, 1, &mImmediateFence, VK_TRUE, UINT64_MAX);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to wait for immediate fence!");
    }
}

#ifndef VULKAN_TYPES
#define VULKAN_TYPES

/*****************************************************
    Includes
*****************************************************/
#include "glm.hpp"
#include "vulkan/vulkan.h"
#include "Vulkan/Include/vma/vk_mem_alloc.h"
#include "Graphics/VulkanDescriptorWriter.h"
#include "Graphics/GraphicsDefine.h"

struct AllocatedBuffer 
{
    // Buffer functions
    void CreateBuffer(VmaAllocator& _allocator, VkDeviceSize _allocSize, VkBufferUsageFlags _usage, VmaMemoryUsage _memoryUsage);
    void DestroyBuffer(VmaAllocator& _allocator);

    // Variables
    VkBuffer buffer{};
    VmaAllocation allocation{};
    VmaAllocationInfo info{};
    VkDeviceSize size{};
    VkDeviceSize capacity{};
};

// For creating image for texture/framebuffer
struct AllocatedImage
{
    // Image functions
    void CreateImage(VkDevice const& _logicalDevice, VmaAllocator& _allocator, VkExtent3D _size, VkFormat _format, VkImageUsageFlags _usage, bool _mipmapped = false);
    void DestroyImage(VkDevice const& _logicalDevice, VmaAllocator& _allocator);

    // Variables
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};

// Vertex info
struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec4 tangent;
};

// Mesh resources
struct GPUMeshBuffers 
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertexBuffer;
    VkDeviceAddress vertexBufferAddress;
    VkDeviceAddress indexBufferAddress;
};

// Push constants for mesh object rendering
struct GPUDrawPushConstants
{
    glm::mat4 worldMatrix;
    glm::mat4 normalMatrix;
};

// For material/shader reflection
struct VulkanMaterialData
{
    VulkanDescriptorWriter writer{};
    AllocatedBuffer uniformBuffer{};
    void* uniformBufferMappedData{};
    size_t uniformBufferSize{};
    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
};

struct GPUPointLight
{
    glm::vec4 position;
    glm::vec4 intensity;
};

struct GPUDirLight
{
    glm::vec4 direction;
    glm::vec4 intensity;
};

// Scene data
struct GPUSceneData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 ambientColorIntensity; // rgb = color, a = intensity
    glm::ivec4 lightCounts; // x = point light count, y = dir light count
    GPUPointLight pointLights[MAX_POINT_LIGHTS];
    GPUDirLight   dirLights[MAX_DIR_LIGHTS];
    glm::vec3 camPos;
    glm::uvec2 fboDims;
    float padding;
};

#endif

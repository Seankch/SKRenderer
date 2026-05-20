#ifndef VULKAN_SHADER
#define VULKAN_SHADER

/*****************************************************
    Includes
*****************************************************/
#include "Vulkan/Include/vma/vk_mem_alloc.h"
#include "Graphics/VulkanDescriptorWriter.h"
#include "Graphics/VulkanTypes.h"
#include <SPIRV-Reflect-main/spirv_reflect.h>
#include <vector>
#include <span>
#include <unordered_map>
#include <optional>

struct VulkanShaderBindings
{
    enum class BindingType
    {
        BT_UNKNOWN = 0,
        BT_UNIFORM_BUFFER,
        BT_UNIFORM_BUFFER_MEMBER,
        BT_COMBINED_IMAGE_SAMPLER,
        BT_TEXTURE,
        BT_SAMPLER
    };

    // Base variables
    std::string name;
    uint32_t bindingIndex{};
    uint32_t size{};
    uint32_t offset{};
    uint32_t setIndex{};
    uint32_t stageFlags{};
    BindingType type = BindingType::BT_UNKNOWN;
    SpvReflectTypeFlags dataType = SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_UNDEFINED;

    // Specific datatype variables
    bool isSigned = false;
    bool isDouble = false;
    bool isTextureIndex = false;
    uint16_t vectorCount = 0;
};

class VulkanShader 
{
public:
    VulkanShader();
    ~VulkanShader();

    // For getting/setting shaders
    void SetShaders(VkShaderModule* _vert, VkShaderModule* _frag);
    VkShaderModule* GetVertShader(void);
    VkShaderModule* GetFragShader(void);

    // For shader reflection
    uint32_t GetMinUBOOffsetAlignment(uint32_t _bufferSize, uint32_t _minUBOOffsetAlignment);
    std::optional<std::vector<VulkanShaderBindings>> ReflectShader(std::span<uint32_t> const& _shaderCode, uint32_t _minUBOOffsetAlignment);
    std::unordered_map<std::string, VulkanShaderBindings> const& GetBindings(void);
    void BuildDescriptorSetLayout(VkDevice _device);
    VkDescriptorSetLayout const& GetDescriptorSetLayout(void);
    uint32_t GetUBOSize(void);
private:
    // Shaders
    VkShaderModule* vertShader;
    VkShaderModule* fragShader;

    // For shader reflection
    VkDescriptorSetLayout descSetLayout{ VK_NULL_HANDLE };
    std::unordered_map<std::string, VulkanShaderBindings> shaderBindings;
    uint32_t uniformBufferSize{};
};

#endif

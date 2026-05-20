#include "Graphics/VulkanShader.h"
#include "Graphics/VulkanDescriptor.h"

VulkanShader::VulkanShader()
: vertShader{}, fragShader{}
{
}

VulkanShader::~VulkanShader()
{

}

void VulkanShader::SetShaders(VkShaderModule* _vert, VkShaderModule* _frag)
{
    vertShader = _vert;
    fragShader = _frag;
}

VkShaderModule* VulkanShader::GetVertShader(void)
{
    return vertShader;
}

VkShaderModule* VulkanShader::GetFragShader(void)
{
    return fragShader;
}

uint32_t VulkanShader::GetMinUBOOffsetAlignment(uint32_t _bufferSize, uint32_t _minUBOOffsetAlignment)
{
	// Compute alignment based on minimum UBO offset alignment
	uint32_t alignedSize = _bufferSize;
	if (_minUBOOffsetAlignment > 0) 
	{
		alignedSize = (alignedSize + _minUBOOffsetAlignment - 1) & ~(_minUBOOffsetAlignment - 1);
	}

	return alignedSize;
}

std::optional<std::vector<VulkanShaderBindings>> VulkanShader::ReflectShader(std::span<uint32_t> const& _shaderCode, uint32_t _minUBOOffsetAlignment)
{
	size_t byteCodeLength = _shaderCode.size() * sizeof(uint32_t);
	SpvReflectShaderModule reflectionModule;
	SpvReflectResult rc = spvReflectCreateShaderModule(byteCodeLength, _shaderCode.data(), &reflectionModule);

	if (rc != SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS)
	{
		return std::nullopt;
	}

	// Bindings list and descriptor count
	std::vector<VulkanShaderBindings> bindings;
	uint32_t descriptorCount{};

	// Get descriptor count, init descriptor bindings
	spvReflectEnumerateDescriptorBindings(&reflectionModule, &descriptorCount, nullptr);
	bindings.reserve(descriptorCount);
	std::vector<SpvReflectDescriptorBinding*> descriptorBindings(descriptorCount);

	// Set shader bindings
	spvReflectEnumerateDescriptorBindings(&reflectionModule, &descriptorCount, descriptorBindings.data());
	for (SpvReflectDescriptorBinding const* descriptor : descriptorBindings)
	{
		// Set 0 and 1 is reserved for scene data and bindless texture
		if (descriptor->set < MATERIAL_MIN_INDEX)
			continue;

		VulkanShaderBindings binding;
		binding.name = descriptor->name;
		binding.setIndex = descriptor->set;
		binding.bindingIndex = descriptor->binding;
		binding.size = descriptor->block.size;
		binding.offset = uniformBufferSize;
		binding.stageFlags = reflectionModule.shader_stage;
		switch (descriptor->descriptor_type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			binding.type = VulkanShaderBindings::BindingType::BT_UNIFORM_BUFFER;
			uniformBufferSize += binding.size;
            uniformBufferSize = GetMinUBOOffsetAlignment(uniformBufferSize, _minUBOOffsetAlignment);
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			binding.type = VulkanShaderBindings::BindingType::BT_COMBINED_IMAGE_SAMPLER;
			binding.dataType = SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE;
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			binding.type = VulkanShaderBindings::BindingType::BT_TEXTURE;
			binding.dataType = SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE;
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			binding.type = VulkanShaderBindings::BindingType::BT_SAMPLER;
			binding.dataType = SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER;
			break;
		default:
			break;
		}
		bindings.emplace_back(binding);

		// Add it to the hash map
		shaderBindings.insert_or_assign(descriptor->name, binding);
		
        // Descriptor isn't a Uniform buffer, skip member reflection
		if (descriptor->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			continue;
		}
		
        // Add UBO member bindings
        uint32_t currentBlockOffset = uniformBufferSize - GetMinUBOOffsetAlignment(binding.size, _minUBOOffsetAlignment);
		for (uint32_t i = 0; i < descriptor->block.member_count; ++i)
		{
			SpvReflectBlockVariable const& member = descriptor->block.members[i];

			// Create a new ShaderBindings entry for each member
			VulkanShaderBindings memberBinding{};
			memberBinding.bindingIndex = descriptor->binding;
			memberBinding.setIndex = descriptor->set;
			memberBinding.size = member.size;
			memberBinding.offset = currentBlockOffset + member.offset;
			memberBinding.name = member.name;
			memberBinding.dataType = member.type_description->type_flags;
			memberBinding.type = VulkanShaderBindings::BindingType::BT_UNIFORM_BUFFER_MEMBER;
			memberBinding.stageFlags = reflectionModule.shader_stage;

			// Set specific datatype members
			memberBinding.isSigned = member.numeric.scalar.signedness;
			memberBinding.isDouble = (member.numeric.scalar.width == 64);
			memberBinding.vectorCount = member.numeric.vector.component_count;
			memberBinding.isTextureIndex = (descriptor->set == MATERIAL_MIN_INDEX) && (descriptor->binding == 1);

			// Add member to binding and add to shader binding map
			bindings.emplace_back(memberBinding);
			shaderBindings.insert_or_assign(member.name, memberBinding);
		}
	}

	// Return shader bindings once done
	return bindings;
}

std::unordered_map<std::string, VulkanShaderBindings> const& VulkanShader::GetBindings(void)
{
	return shaderBindings;
}

void VulkanShader::BuildDescriptorSetLayout(VkDevice _device)
{
	if (descSetLayout != VK_NULL_HANDLE)
		return;

	VulkanDescriptor descriptor{};
	std::unordered_map<std::string, VulkanShaderBindings>::iterator iter = shaderBindings.begin();
	for (iter; iter != shaderBindings.end(); ++iter)
	{
		// Set 0 and 1 is reserved for scene data and bindless texture
		if (iter->second.setIndex < MATERIAL_MIN_INDEX)
			continue;

		// Add bindings
		switch (iter->second.type)
		{
		case VulkanShaderBindings::BindingType::BT_UNIFORM_BUFFER:
			descriptor.AddBinding(iter->second.bindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			break;
		case VulkanShaderBindings::BindingType::BT_COMBINED_IMAGE_SAMPLER:
			descriptor.AddBinding(iter->second.bindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			break;
		case VulkanShaderBindings::BindingType::BT_TEXTURE:
			descriptor.AddBinding(iter->second.bindingIndex, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			break;
		case VulkanShaderBindings::BindingType::BT_SAMPLER:
			descriptor.AddBinding(iter->second.bindingIndex, VK_DESCRIPTOR_TYPE_SAMPLER);
			break;
		default:
			break;
		}
	}

	// Build descriptor set layout
	descSetLayout = descriptor.Build(_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

VkDescriptorSetLayout const& VulkanShader::GetDescriptorSetLayout(void)
{
	return descSetLayout;
}

uint32_t VulkanShader::GetUBOSize(void)
{
	return uniformBufferSize;
}

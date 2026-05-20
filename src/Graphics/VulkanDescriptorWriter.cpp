#include "Graphics/VulkanDescriptorWriter.h"

void VulkanDescriptorWriter::WriteImage(int _binding, VkImageView _image, VkSampler _sampler, VkImageLayout _layout, VkDescriptorType _type, uint32_t _dstArrayElement)
{
	VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo
	{
		.sampler = _sampler,
		.imageView = _image,
		.imageLayout = _layout
	});

	VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstBinding = _binding;
	write.dstSet = VK_NULL_HANDLE;
	write.dstArrayElement = _dstArrayElement;
	write.descriptorCount = 1;
	write.descriptorType = _type;
	write.pImageInfo = &info;
	writes.push_back(write);
}

void VulkanDescriptorWriter::WriteBuffer(int _binding, VkBuffer _buffer, size_t _size, size_t _offset, VkDescriptorType _type)
{
	VkDescriptorBufferInfo& info = bufferInfos.emplace_back(VkDescriptorBufferInfo
	{
		.buffer = _buffer,
		.offset = _offset,
		.range = _size
	});

	VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstBinding = _binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = _type;
	write.pBufferInfo = &info;
	writes.push_back(write);
}

void VulkanDescriptorWriter::WriteAccelerationStructure(int _binding, VkAccelerationStructureKHR* _tlas)
{
	VkWriteDescriptorSetAccelerationStructureKHR& asInfo = asInfos.emplace_back();
	asInfo = {};
	asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	asInfo.accelerationStructureCount = 1;
	asInfo.pAccelerationStructures = &(*_tlas);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = &asInfo;
	write.dstBinding = _binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	writes.push_back(write);
}

void VulkanDescriptorWriter::Clear()
{
	imageInfos.clear();
	writes.clear();
	bufferInfos.clear();
}

void VulkanDescriptorWriter::UpdateSet(VkDevice _device, VkDescriptorSet _set)
{
	for (VkWriteDescriptorSet& write : writes) 
	{
		write.dstSet = _set;
	}

	vkUpdateDescriptorSets(_device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

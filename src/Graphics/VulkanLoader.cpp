#include <Graphics/VulkanLoader.h>

void VulkanLoader::LoadFunctions(VkDevice _logicalDevice)
{
    // Load extension functions
    vklCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkCreateAccelerationStructureKHR"));
    vklGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
    vklCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
    vklGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
    vklGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkGetBufferDeviceAddressKHR"));
    vklDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(_logicalDevice, "vkDestroyAccelerationStructureKHR"));
}

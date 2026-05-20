#ifndef VULKAN_LOADER
#define VULKAN_LOADER

/*****************************************************
    Includes
*****************************************************/
#include "vulkan/vulkan.h"

class VulkanLoader
{
public:
    // For loading of extension functions
    void LoadFunctions(VkDevice _logicalDevice);

    // Loaded function ptrs
    PFN_vkCreateAccelerationStructureKHR              vklCreateAccelerationStructureKHR = nullptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR       vklGetAccelerationStructureBuildSizesKHR = nullptr;
    PFN_vkCmdBuildAccelerationStructuresKHR           vklCmdBuildAccelerationStructuresKHR = nullptr;
    PFN_vkGetAccelerationStructureDeviceAddressKHR    vklGetAccelerationStructureDeviceAddressKHR = nullptr;
    PFN_vkGetBufferDeviceAddressKHR                   vklGetBufferDeviceAddressKHR = nullptr;
    PFN_vkDestroyAccelerationStructureKHR             vklDestroyAccelerationStructureKHR = nullptr; 
};

#endif
//
// Created by Kiril on 17.04.2023.
//

#ifndef DEEPLOM_UTILS_HPP
#define DEEPLOM_UTILS_HPP

#include <vulkan/vulkan_raii.hpp>

namespace vk_utils
{

uint32_t memory_type(const vk::raii::PhysicalDevice& phys_device, vk::MemoryPropertyFlags properties, uint32_t filter)
{
	vk::PhysicalDeviceMemoryProperties mp = phys_device.getMemoryProperties();
	for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
	{
		if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("No suitable memory type found!");
}

} // vk_utils

#endif //DEEPLOM_UTILS_HPP

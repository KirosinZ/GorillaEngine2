//
// Created by Kiril on 24.04.2023.
//

#include "physical_device_helpers.hpp"


namespace vk_helpers
{

vk::raii::PhysicalDevice most_suitable_device(const vk::raii::PhysicalDevices& phys_devices)
{
	return phys_devices[0];
}

vk::raii::PhysicalDevice most_suitable_device(const std::vector<vk::raii::PhysicalDevice>& phys_devices)
{
	return phys_devices[0];
}


std::vector<int32_t> queue_families(const std::vector<vk::QueueFamilyProperties>& families, vk::QueueFlags flags)
{
	std::vector<int32_t> res;
	for (int i = 0; i < families.size(); i++)
		if (families[i].queueFlags & flags)
			res.push_back(i);

	return res;
}

std::vector<int32_t> present_families(
		const std::vector<vk::QueueFamilyProperties>& families,
		const vk::raii::PhysicalDevice& phys_device,
		const vk::raii::SurfaceKHR& surface)
{
	std::vector<int32_t> res;
	for (int i = 0; i < families.size(); i++)
		if (phys_device.getSurfaceSupportKHR(i, *surface))
			res.push_back(i);

	return res;
}

} // vk_helpers
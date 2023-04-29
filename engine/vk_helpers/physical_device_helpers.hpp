//
// Created by Kiril on 24.04.2023.
//

#ifndef DEEPLOM_PHYSICAL_DEVICE_HELPERS_HPP
#define DEEPLOM_PHYSICAL_DEVICE_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>


namespace vk_helpers
{

vk::raii::PhysicalDevice most_suitable_device(const vk::raii::PhysicalDevices& phys_devices);
vk::raii::PhysicalDevice most_suitable_device(const std::vector<vk::raii::PhysicalDevice>& phys_devices);

std::vector<int32_t> queue_families(const std::vector<vk::QueueFamilyProperties>& families, vk::QueueFlags flags);
std::vector<int32_t> present_families(
		const std::vector<vk::QueueFamilyProperties>& families,
		const vk::raii::PhysicalDevice& phys_device,
		const vk::raii::SurfaceKHR& surface);



} // vk_helpers

#endif //DEEPLOM_PHYSICAL_DEVICE_HELPERS_HPP

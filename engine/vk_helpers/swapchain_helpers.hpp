//
// Created by Kiril on 27.04.2023.
//

#ifndef DEEPLOM_SWAPCHAIN_HELPERS_HPP
#define DEEPLOM_SWAPCHAIN_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>


namespace vk_helpers
{

struct swapchain_support_details
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> present_models;
};

swapchain_support_details support_details(
		const vk::raii::PhysicalDevice& phys_device,
		const vk::raii::SurfaceKHR& surface);

}

#endif //DEEPLOM_SWAPCHAIN_HELPERS_HPP

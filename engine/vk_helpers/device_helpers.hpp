#pragma once

#include <vulkan/vulkan_raii.hpp>


namespace gorilla::vk_helpers
{

vk::raii::Device device(
		const vk::raii::PhysicalDevice& phys_device,
		const std::vector<vk::DeviceQueueCreateInfo>& queue_create_infos,
		const std::vector<const char*>& device_extensions = {},
		const vk::PhysicalDeviceFeatures& features = {});

}

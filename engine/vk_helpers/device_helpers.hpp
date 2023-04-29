//
// Created by Kiril on 25.04.2023.
//

#ifndef DEEPLOM_DEVICE_HELPERS_HPP
#define DEEPLOM_DEVICE_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>


namespace vk_helpers
{

vk::DeviceQueueCreateInfo queue_create_info(int32_t index, const std::vector<float>& priorities);

vk::raii::Device device(
		const vk::raii::PhysicalDevice& phys_device,
		const std::vector<vk::DeviceQueueCreateInfo>& queue_create_infos,
		const std::vector<const char*>& device_extensions = {},
		const vk::PhysicalDeviceFeatures& features = {});

} // vk_helpers

#endif //DEEPLOM_DEVICE_HELPERS_HPP

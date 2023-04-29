//
// Created by Kiril on 24.04.2023.
//

#include "environment_helpers.hpp"

#include "instance_helpers.hpp"
#include "physical_device_helpers.hpp"
#include "device_helpers.hpp"


namespace vk_helpers
{

vk_utils::environment headless_env(
		const std::string& name,
		uint32_t app_version,
		const std::vector<const char*>& instance_layers,
		const std::vector<const char*>& instance_extensions,
		const std::vector<const char*>& device_extensions)
{
	vk::raii::Instance instance = vk_helpers::instance(
			name,
			app_version,
			instance_layers,
			instance_extensions);

	vk::raii::PhysicalDevice physical_device = vk_helpers::most_suitable_device(instance.enumeratePhysicalDevices());

	std::vector<vk::QueueFamilyProperties> queue_properties = physical_device.getQueueFamilyProperties();
	int32_t graphics = vk_helpers::queue_families(queue_properties, vk::QueueFlagBits::eGraphics)[0];

	std::vector<float> priorities = { 1.0f };
	std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos = {
			vk_helpers::queue_create_info(graphics, priorities),
	};
	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	vk::raii::Device device = vk_helpers::device(
			physical_device,
			device_queue_create_infos,
			device_extensions,
			features);

	vk::raii::Queue graphics_queue = device.getQueue(graphics, 0);

	return {
		std::move(instance),
		std::move(physical_device),
		std::move(device),
		graphics,
		std::move(graphics_queue)
	};
}

vk_utils::environment headful_env(
		const gorilla::glfw::window& window,
		const std::string& name,
		uint32_t app_version,
		const std::vector<const char*>& instance_layers,
		const std::vector<const char*>& instance_extensions,
		const std::vector<const char*>& device_extensions)
{
	std::vector<const char*> instance_extensions_window = instance_extensions;
	std::vector<const char*> window_extensions = gorilla::glfw::window::extensions();
	for (auto window_extension : window_extensions)
	{
		instance_extensions_window.push_back(window_extension);
	}

	vk::raii::Instance instance = vk_helpers::instance(
			name,
			app_version,
			instance_layers,
			instance_extensions_window);

	std::vector<vk::raii::SurfaceKHR> surfaces;
	surfaces.emplace_back(window.surface(instance));

	vk::raii::PhysicalDevice physical_device = vk_helpers::most_suitable_device(instance.enumeratePhysicalDevices());

	std::vector<vk::QueueFamilyProperties> queue_properties = physical_device.getQueueFamilyProperties();
	int32_t graphics = vk_helpers::queue_families(queue_properties, vk::QueueFlagBits::eGraphics)[0];
	int32_t present = vk_helpers::present_families(queue_properties, physical_device, surfaces[0])[1];

	std::vector<float> priorities = { 1.0f };
	std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos = {
			vk_helpers::queue_create_info(graphics, priorities),
			vk_helpers::queue_create_info(present, priorities),
	};

	std::vector<const char*> device_extensions_swapchain = device_extensions;
	device_extensions_swapchain.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	vk::raii::Device device = vk_helpers::device(
			physical_device,
			device_queue_create_infos,
			device_extensions_swapchain,
			features);

	vk::raii::Queue graphics_queue = device.getQueue(graphics, 0);
	vk::raii::Queue present_queue = device.getQueue(present, 0);

	return {
			std::move(instance),
			std::move(physical_device),
			std::move(device),
			graphics,
			std::move(graphics_queue),
			std::move(surfaces),
			present,
			std::move(present_queue),
	};
}

} // vk_helpers
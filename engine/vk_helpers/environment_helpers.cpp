#include "environment_helpers.hpp"

#include "instance_helpers.hpp"
#include "physical_device_helpers.hpp"
#include "queue_helpers.hpp"
#include "device_helpers.hpp"


namespace gorilla::vk_helpers
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

	std::vector queue_families = vk_helpers::queue_families(instance, physical_device);
	vk_utils::queue_family graphics_family = *std::ranges::find_if(queue_families, &vk_utils::queue_family::has_graphics);

	std::array priorities{ 1.0f };
	std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos = {
			vk::DeviceQueueCreateInfo({}, graphics_family.index(), priorities)
	};
	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	vk::raii::Device device = vk_helpers::device(
			physical_device,
			device_queue_create_infos,
			device_extensions,
			features);

	return {
		std::move(instance),
		std::move(physical_device),
		std::move(device),
		std::vector{
			graphics_family
		},
		vk_helpers::get_queue(device, graphics_family)
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

	std::vector queue_families = vk_helpers::queue_families(instance, physical_device);
	vk_utils::queue_family graphics_family = *std::ranges::find_if(queue_families, &vk_utils::queue_family::has_all_graphics);

	std::vector<float> priorities = { 1.0f };
	std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos = {
		//TODO: later...
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

	return {
			std::move(instance),
			std::move(physical_device),
			std::move(device),
		std::vector{
			graphics_family
		},
			vk_helpers::get_queue(device, graphics_family)
	};
}

}
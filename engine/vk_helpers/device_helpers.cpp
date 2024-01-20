#include "device_helpers.hpp"


namespace gorilla::vk_helpers
{

vk::raii::Device device(
		const vk::raii::PhysicalDevice& phys_device,
		const std::vector<vk::DeviceQueueCreateInfo>& queue_create_infos,
		const std::vector<const char*>& device_extensions,
		const vk::PhysicalDeviceFeatures& features)
{
	const vk::DeviceCreateInfo create_info(
			{},
			queue_create_infos,
			{},
			device_extensions,
			&features);

	return phys_device.createDevice(create_info);
}

}
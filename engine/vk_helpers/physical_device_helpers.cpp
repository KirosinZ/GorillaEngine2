#include "physical_device_helpers.hpp"


namespace gorilla::vk_helpers
{

vk::raii::PhysicalDevice most_suitable_device(const vk::raii::PhysicalDevices& phys_devices)
{
	return phys_devices[0];
}

vk::raii::PhysicalDevice most_suitable_device(const std::vector<vk::raii::PhysicalDevice>& phys_devices)
{
	return phys_devices[0];
}

}
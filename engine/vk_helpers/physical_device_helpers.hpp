#pragma once

#include <vulkan/vulkan_raii.hpp>


namespace gorilla::vk_helpers
{

vk::raii::PhysicalDevice most_suitable_device(const vk::raii::PhysicalDevices& phys_devices);
vk::raii::PhysicalDevice most_suitable_device(const std::vector<vk::raii::PhysicalDevice>& phys_devices);

}
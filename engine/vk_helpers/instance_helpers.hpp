//
// Created by Kiril on 24.04.2023.
//

#ifndef DEEPLOM_INSTANCE_HELPERS_HPP
#define DEEPLOM_INSTANCE_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>


namespace vk_helpers
{

vk::ApplicationInfo application_info(
		const std::string& app_name = "Application",
		uint32_t app_version = VK_MAKE_VERSION(0, 1, 0),
		const std::string& engine_name = "Gorilla Engine",
		uint32_t engine_version = VK_MAKE_VERSION(0, 1, 0),
		uint32_t api_version = VK_API_VERSION_1_3);

vk::raii::Instance instance(
		const vk::ApplicationInfo& app_info,
		const std::vector<const char*>& layers = {},
		const std::vector<const char*>& extensions = {});

vk::raii::Instance instance(
		const std::string& name = "Application",
		uint32_t app_version = VK_MAKE_VERSION(1, 0, 0),
		const std::vector<const char*>& layers = {},
		const std::vector<const char*>& extensions = {});

} // vk_helpers

#endif //DEEPLOM_INSTANCE_HELPERS_HPP

//
// Created by Kiril on 24.04.2023.
//

#include "instance_helpers.hpp"

#include <vk_utils/environment.hpp>


namespace vk_helpers
{

vk::ApplicationInfo application_info(
		const std::string& app_name,
		uint32_t app_version,
		const std::string& engine_name,
		uint32_t engine_version,
		uint32_t api_version)
{
	vk::ApplicationInfo res(
			app_name.c_str(),
			app_version,
			engine_name.c_str(),
			engine_version,
			api_version);

	return res;
}

vk::raii::Instance instance(
		const vk::ApplicationInfo& app_info,
		const std::vector<const char*>& layers,
		const std::vector<const char*>& extensions)
{
	const vk::InstanceCreateInfo create_info(
			{},
			&app_info,
			layers,
			extensions);
	const auto& context = vk_utils::environment::context();

	return context.createInstance(create_info);
}

vk::raii::Instance instance(
		const std::string& name,
		uint32_t app_version,
		const std::vector<const char*>& layers,
		const std::vector<const char*>& extensions)
{
	const vk::ApplicationInfo app_info = application_info(name, app_version);

	return instance(app_info, layers, extensions);
}

} // vk_helpers
#pragma once

#include <vk_utils/environment.hpp>
#include <engine/window/window.hpp>

namespace gorilla::vk_helpers
{

vk_utils::environment headless_env(
		const std::string& name = "Application",
		uint32_t app_version = VK_MAKE_VERSION(1, 0, 0),
		const std::vector<const char*>& instance_layers = {},
		const std::vector<const char*>& instance_extensions = {},
		const std::vector<const char*>& device_extensions = {});

vk_utils::environment headful_env(
		const gorilla::glfw::window& window,
		const std::string& name = "Application",
		uint32_t app_version = VK_MAKE_VERSION(1, 0, 0),
		const std::vector<const char*>& instance_layers = {},
		const std::vector<const char*>& instance_extensions = {},
		const std::vector<const char*>& device_extensions = {});


}

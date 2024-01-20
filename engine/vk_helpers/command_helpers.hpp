#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_utils/environment.hpp>

namespace gorilla::vk_helpers
{

vk::raii::CommandBuffer single_time_command(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool);

void end_command(
		const vk_utils::environment& env,
		const vk::raii::CommandBuffer& cmd);

}

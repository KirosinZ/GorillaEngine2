//
// Created by Kiril on 29.04.2023.
//

#ifndef DEEPLOM_COMMAND_HELPERS_HPP
#define DEEPLOM_COMMAND_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>
#include <vk_utils/environment.hpp>

namespace vk_helpers
{

vk::raii::CommandBuffer single_time_command(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool);

void end_command(
		const vk_utils::environment& env,
		const vk::raii::CommandBuffer& cmd);

}

#endif //DEEPLOM_COMMAND_HELPERS_HPP

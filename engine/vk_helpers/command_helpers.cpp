#include "command_helpers.hpp"


namespace gorilla::vk_helpers
{

vk::raii::CommandBuffer single_time_command(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool)
{
	const vk::raii::Device& device = env.device();

	vk::CommandBufferAllocateInfo allocate_info(
			*cmd_pool,
			vk::CommandBufferLevel::ePrimary,
			1);
	vk::raii::CommandBuffer cmd = std::move(device.allocateCommandBuffers(allocate_info)[0]);

	vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cmd.begin(begin_info);

	return cmd;
}

void end_command(
		const vk_utils::environment& env,
		const vk::raii::CommandBuffer& cmd)
{
	const vk::raii::Queue& graphics_queue = env.graphics_queue().vk_queue();

	cmd.end();

	vk::SubmitInfo submit_info({}, {}, *cmd);
	graphics_queue.submit(submit_info);
	graphics_queue.waitIdle();
}

}
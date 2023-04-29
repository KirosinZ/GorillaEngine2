//
// Created by Kiril on 29.04.2023.
//

#include "buffer_helpers.hpp"
#include "command_helpers.hpp"

namespace vk_helpers
{

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> buffer(
		const vk_utils::environment& env,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags mem_properties)
{
	const vk::raii::Device& device = env.device();

	vk::BufferCreateInfo create_info(
			{},
			size,
			usage,
			vk::SharingMode::eExclusive);
	vk::raii::Buffer buffer = device.createBuffer(create_info);

	vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();

	vk::MemoryAllocateInfo allocate_info(
			mem_requirements.size,
			memory_type(
					env,
					mem_requirements.memoryTypeBits,
					mem_properties));
	vk::raii::DeviceMemory memory = device.allocateMemory(allocate_info);

	buffer.bindMemory(*memory, 0);

	return {
		std::move(buffer),
		std::move(memory)
	};
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> staging_buffer(
		const vk_utils::environment& env,
		vk::DeviceSize size)
{
	return buffer(
			env,
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void copy_buffer(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Buffer& src_buffer,
		const vk::raii::Buffer& dst_buffer,
		vk::DeviceSize size)
{
	vk::raii::CommandBuffer cmd = single_time_command(env, cmd_pool);

	vk::BufferCopy copy(0, 0, size);
	cmd.copyBuffer(
			*src_buffer,
			*dst_buffer,
			copy);

	end_command(env, cmd);
}

void copy_buffer_to_image(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Buffer& src_buffer,
		const vk::raii::Image& dst_image,
		uint32_t width,
		uint32_t height)
{
	vk::raii::CommandBuffer cmd = single_time_command(env, cmd_pool);

	vk::BufferImageCopy copy(
			0,
			0,
			0,
			{ vk::ImageAspectFlagBits::eColor, 0, 0, 1},
			{ 0, 0, 0 },
			{ width, height, 1 });

	cmd.copyBufferToImage(
			*src_buffer,
			*dst_image,
			vk::ImageLayout::eTransferDstOptimal,
			copy);

	end_command(env, cmd);
}

uint32_t memory_type(
		const vk_utils::environment& env,
		uint32_t filter,
		vk::MemoryPropertyFlags mem_properties)
{
	const vk::raii::PhysicalDevice& phys_device = env.phys_device();

	vk::PhysicalDeviceMemoryProperties mem_properties_actual = phys_device.getMemoryProperties();
	for (int i = 0; i < mem_properties_actual.memoryTypeCount; ++i)
		if (filter & (1 << i) && (mem_properties_actual.memoryTypes[i].propertyFlags & mem_properties) == mem_properties)
			return i;

	return -1;
}

}



#include "buffer_helpers.hpp"

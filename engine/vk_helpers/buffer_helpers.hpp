//
// Created by Kiril on 29.04.2023.
//

#ifndef DEEPLOM_BUFFER_HELPERS_HPP
#define DEEPLOM_BUFFER_HELPERS_HPP

#include <vulkan/vulkan_raii.hpp>
#include <vk_utils/environment.hpp>

namespace vk_helpers
{

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> buffer(
		const vk_utils::environment& env,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags mem_properties);

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> staging_buffer(
		const vk_utils::environment& env,
		vk::DeviceSize size);

void copy_buffer(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Buffer& src_buffer,
		const vk::raii::Buffer& dst_buffer,
		vk::DeviceSize size);

void copy_buffer_to_image(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Buffer& src_buffer,
		const vk::raii::Image& dst_image,
		uint32_t width,
		uint32_t height);

uint32_t memory_type(
		const vk_utils::environment& env,
		uint32_t filter,
		vk::MemoryPropertyFlags mem_properties);

}

#endif //DEEPLOM_BUFFER_HELPERS_HPP

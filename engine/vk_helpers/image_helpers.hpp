#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_utils/environment.hpp>

namespace gorilla::vk_helpers
{

std::pair<vk::raii::Image, vk::raii::DeviceMemory> image(
		const vk_utils::environment& env,
		uint32_t width,
		uint32_t height,
		uint32_t mip_levels,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags mem_properties);

vk::raii::ImageView image_view(
		const vk_utils::environment& env,
		const vk::raii::Image& image,
		vk::Format format,
		uint32_t mip_levels);

void transition_image_layout(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Image& image,
		vk::Format format,
		vk::ImageLayout old_layout,
		vk::ImageLayout new_layout,
		uint32_t mip_levels);

void generate_mipmaps(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Image& image,
		vk::Format format,
		int32_t width,
		int32_t height,
		uint32_t mip_levels);

}

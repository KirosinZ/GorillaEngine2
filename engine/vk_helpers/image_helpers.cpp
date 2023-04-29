//
// Created by Kiril on 29.04.2023.
//

#include <iostream>
#include "image_helpers.hpp"
#include "buffer_helpers.hpp"
#include "command_helpers.hpp"


namespace vk_helpers
{

std::pair<vk::raii::Image, vk::raii::DeviceMemory> image(
		const vk_utils::environment& env,
		uint32_t width,
		uint32_t height,
		uint32_t mip_levels,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags mem_properties)
{
	const vk::raii::Device& device = env.device();

	vk::ImageCreateInfo create_info(
			{},
			vk::ImageType::e2D,
			format,
			{ width, height, 1 },
			mip_levels,
			1,
			vk::SampleCountFlagBits::e1,
			tiling,
			usage,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined);
	vk::raii::Image image = device.createImage(create_info);

	vk::DeviceImageMemoryRequirements image_mem_requirements(&create_info);

	vk::MemoryRequirements2 mem_requirements = device.getImageMemoryRequirements(image_mem_requirements);

	vk::MemoryAllocateInfo allocate_info(
			mem_requirements.memoryRequirements.size,
			memory_type(
					env,
					mem_requirements.memoryRequirements.memoryTypeBits,
					mem_properties));
	vk::raii::DeviceMemory memory = device.allocateMemory(allocate_info);

	vk::BindImageMemoryInfo bind_info(
			*image,
			*memory,
			0);
	device.bindImageMemory2(bind_info);

	return {
		std::move(image),
		std::move(memory)
	};
}

vk::raii::ImageView image_view(
		const vk_utils::environment& env,
		const vk::raii::Image& image,
		vk::Format format,
		uint32_t mip_levels)
{
	const vk::raii::Device& device = env.device();

	vk::ImageViewCreateInfo create_info(
			{},
			*image,
			vk::ImageViewType::e2D,
			format,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					mip_levels,
					0,
					1));
	return device.createImageView(create_info);
}

void transition_image_layout(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Image& image,
		vk::Format format,
		vk::ImageLayout old_layout,
		vk::ImageLayout new_layout,
		uint32_t mip_levels)
{
	vk::raii::CommandBuffer cmd = single_time_command(env, cmd_pool);

	vk::ImageMemoryBarrier barrier(
			{},
			{},
			old_layout,
			new_layout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			*image,
			{ vk::ImageAspectFlagBits::eColor, 0, mip_levels, 0, 1 });

	if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

//		if (hasStencilComponent(format))
//		{
//			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
//		}
	}

	vk::PipelineStageFlags src_stage;
	vk::PipelineStageFlags dst_stage;
	if (old_layout == vk::ImageLayout::eUndefined
	    && new_layout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		dst_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (old_layout == vk::ImageLayout::eTransferDstOptimal
	         && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		src_stage = vk::PipelineStageFlagBits::eTransfer;
		dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (old_layout == vk::ImageLayout::eUndefined
	         && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		std::cerr << "Bad image layout transition" << std::endl;
		terminate();
	}

	cmd.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);

	end_command(env, cmd);
}

void generate_mipmaps(
		const vk_utils::environment& env,
		const vk::raii::CommandPool& cmd_pool,
		const vk::raii::Image& image,
		vk::Format format,
		int32_t width,
		int32_t height,
		uint32_t mip_levels)
{
	const vk::raii::PhysicalDevice& phys_device = env.phys_device();

	vk::FormatProperties properties = phys_device.getFormatProperties(format);
	if (!(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		std::cerr << "image format does not support linear blitting" << std::endl;
		terminate();
	}

	vk::raii::CommandBuffer cmd = single_time_command(env, cmd_pool);

	vk::ImageMemoryBarrier barrier(
			{},
			{},
			{},
			{},
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			*image,
			{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

	int32_t mip_width = width;
	int32_t mip_height = height;

	for (int32_t i = 1; i < mip_levels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eTransfer,
				{},
				{},
				{},
				barrier);

		std::array<vk::Offset3D, 2> src_offsets = {
				vk::Offset3D(0, 0, 0),
				vk::Offset3D(mip_width, mip_height, 1)
		};
		std::array<vk::Offset3D, 2> dst_offsets = {
				vk::Offset3D(0, 0, 0),
				vk::Offset3D(mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1)
		};

		vk::ImageBlit blit(
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1),
				src_offsets,
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1),
				dst_offsets);
		cmd.blitImage(
				*image,
				vk::ImageLayout::eTransferSrcOptimal,
				*image,
				vk::ImageLayout::eTransferDstOptimal,
				blit,
				vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				{},
				{},
				{},
				barrier);

		if (mip_width > 1) mip_width /= 2;
		if (mip_height > 1) mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{},
			{},
			{},
			barrier);

	end_command(env, cmd);
}

}
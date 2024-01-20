#include "applicationsimple.hpp"

namespace gorilla {

application_simple::application_simple()
{
	std::ranges::transform(_sc_images, std::back_inserter(_sc_image_views), [&] (const vk::Image& img) -> vk::raii::ImageView {
		vk::ImageViewCreateInfo ivci(
			{},
			img,
			vk::ImageViewType::e2D,
			_sc_format,
			vk::ComponentMapping{},
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1));
		return _device.createImageView(ivci);
	});

	_depth_format = vk::Format::eD24UnormS8Uint;
	std::vector queue_indices{
		_graphics_family.index()
	};
	vk::ImageCreateInfo ici(
		{},
		vk::ImageType::e2D,
		_depth_format,
		vk::Extent3D{ _sc_extent, 1 },
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::SharingMode::eExclusive,
		queue_indices,
		vk::ImageLayout::eUndefined);
	_depth_image = _device.createImage(ici);

	vk::ImageMemoryRequirementsInfo2 imri(*_depth_image);
	vk::MemoryRequirements mreqs = _device.getImageMemoryRequirements2(imri).memoryRequirements;

	uint32_t type_index = 0;
	uint32_t index = 0;
	for (auto mt: _memory_types)
	{
		if (mt.is_device_local() && (mreqs.memoryTypeBits & (1 << index++)) != 0)
		{
			type_index = mt.index();
			break;
		}
	}
	vk::MemoryAllocateInfo mai(mreqs.size, type_index);
	_depth_memory = _device.allocateMemory(mai);

	vk::BindImageMemoryInfo bimi(*_depth_image, *_depth_memory);
	_device.bindImageMemory2(bimi);

	vk::ImageViewCreateInfo ivci(
		{},
		*_depth_image,
		vk::ImageViewType::e2D,
		_depth_format,
		vk::ComponentMapping{},
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eDepth,
			0,
			1,
			0,
			1));
	_depth_image_view = _device.createImageView(ivci);

	std::vector attachments{
		vk::AttachmentDescription(
			{},
			_sc_format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR),
		vk::AttachmentDescription(
			{},
			_depth_format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal),
	};

	vk::AttachmentReference color_attachment(
		0,
		vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depth_attachment(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription sd(
		{},
		vk::PipelineBindPoint::eGraphics,
		{},
		color_attachment,
		{},
		&depth_attachment,
		{});

	vk::SubpassDependency sdep(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		{},
		vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	vk::RenderPassCreateInfo rpci(
		{},
		attachments,
		sd,
		sdep);
	_render_pass = _device.createRenderPass(rpci);

	for (int i_ind = 0; i_ind < _image_count; i_ind++)
	{
		std::vector fb_attachments{
			*_sc_image_views[i_ind],
			*_depth_image_view
		};
		vk::FramebufferCreateInfo fbci(
			{},
			*_render_pass,
			fb_attachments,
			_sc_extent.width,
			_sc_extent.height,
			1);
		_framebuffers.emplace_back(_device, fbci);
	}
}


void application_simple::process_input()
{
	glfw::window::poll_events();

	if (_window.get_key(glfw::key_id::escape) == glfw::key_action::press)
		_window.set_close(true);
}

void application_simple::pre_render()
{

}

void application_simple::render(
	const vk::raii::CommandBuffer& cmd,
	uint32_t img_index)
{
	std::vector<vk::ClearValue> clear_values{
		vk::ClearColorValue(std::array{ 1.0f, 0.2f, 0.2f, 0.0f }),
		vk::ClearDepthStencilValue(1.0f, 0)
	};
	vk::RenderPassBeginInfo rpbi(
		*_render_pass,
		*_framebuffers[img_index],
		vk::Rect2D(vk::Offset2D{ 0, 0 }, _sc_extent),
		clear_values);
	cmd.beginRenderPass(rpbi, vk::SubpassContents::eInline);



	cmd.endRenderPass();
}


}
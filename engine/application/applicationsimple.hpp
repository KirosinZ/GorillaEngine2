#pragma once

#include "applicationbase.hpp"


namespace gorilla {

class application_simple : public application_base
{
public:
	application_simple();

protected:
	vk::raii::RenderPass _render_pass = nullptr;
	std::vector<vk::raii::ImageView> _sc_image_views;

	vk::Format _depth_format;
	vk::raii::Image _depth_image = nullptr;
	vk::raii::DeviceMemory _depth_memory = nullptr;
	vk::raii::ImageView _depth_image_view = nullptr;

	std::vector<vk::raii::Framebuffer> _framebuffers;


	void process_input() override;
	void pre_render() override;

	void render(const vk::raii::CommandBuffer& cmd, uint32_t img_index) override;
};

}

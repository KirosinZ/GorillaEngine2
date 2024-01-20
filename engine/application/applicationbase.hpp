#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <window/window.hpp>

#include "memory_type.hpp"
#include "../vk_utils/queue_family.hpp"


namespace gorilla {

class application_base {
public:
	virtual ~application_base() = default;

	application_base();

	void run();

protected:
	glfw::window _window;

	vk::raii::Instance _instance = nullptr;
	vk::raii::PhysicalDevice _phys_device = nullptr;
	std::vector<vk_utils::queue_family> _queue_families;
	std::vector<memory_type> _memory_types;
	vk::raii::Device _device = nullptr;

	vk_utils::queue_family _graphics_family;
	vk::raii::Queue _graphics_queue = nullptr;

	vk_utils::queue_family _present_family;
	vk::raii::Queue _present_queue = nullptr;

	vk_utils::queue_family _transfer_family;
	vk::raii::Queue _transfer_queue = nullptr;

	vk::raii::SurfaceKHR _surface = nullptr;
	vk::raii::SwapchainKHR _swapchain = nullptr;
	std::vector<vk::Image> _sc_images;

	std::vector<vk::SurfaceFormatKHR> _surface_formats;
	vk::SurfaceCapabilitiesKHR _surface_capabilities;

	vk::Format _sc_format;
	vk::ColorSpaceKHR _sc_cspace;
	vk::Extent2D _sc_extent;

	vk::raii::CommandPool _command_pool = nullptr;
	std::vector<vk::raii::CommandBuffer> _frame_cmds;

	std::vector<vk::raii::Fence> _frame_in_flight_fences;
	std::vector<vk::raii::Semaphore> _image_available_semaphores;
	std::vector<vk::raii::Semaphore> _render_finished_semaphores;

	uint32_t _current_frame = 0;
	uint32_t _image_count = 0;

	virtual void process_input() = 0;
	virtual void pre_render() = 0;
	virtual void render(const vk::raii::CommandBuffer& cmd, uint32_t img_index) = 0;
};

} // gorilla

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "queue.hpp"


namespace gorilla::vk_utils
{

class environment
{
public:
	environment() = default;
	environment(
			vk::raii::Instance&& instance,
			vk::raii::PhysicalDevice&& phys_device,
			vk::raii::Device&& device,
			std::vector<queue_family> available_queues,
			queue&& graphics_queue);

	[[nodiscard]] const vk::raii::Instance& instance() const { return _instance; }
	[[nodiscard]] const vk::raii::PhysicalDevice& phys_device() const { return _phys_device; }
	[[nodiscard]] const vk::raii::Device& device() const { return _device; }
	[[nodiscard]] const std::vector<queue_family>& available_families() const { return _available_queues; }
	[[nodiscard]] const queue& graphics_queue() const { return _graphics_queue; }
private:
	vk::raii::Instance _instance = nullptr;
	vk::raii::PhysicalDevice _phys_device = nullptr;
	vk::raii::Device _device = nullptr;
	std::vector<queue_family> _available_queues;

	queue _graphics_queue;
};

}

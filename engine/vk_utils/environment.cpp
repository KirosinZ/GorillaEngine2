#include "environment.hpp"

namespace gorilla::vk_utils
{

environment::environment(
		vk::raii::Instance&& instance,
		vk::raii::PhysicalDevice&& phys_device,
		vk::raii::Device&& device,
		std::vector<queue_family> available_queues,
		queue&& graphics_queue) :
	_instance(std::move(instance)),
	_phys_device(std::move(phys_device)),
	_device(std::move(device)),
	_available_queues(std::move(available_queues)),
	_graphics_queue(std::move(graphics_queue))
{}

}
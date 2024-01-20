#include "queue_helpers.hpp"

#include <window/window.hpp>


namespace gorilla::vk_helpers
{

std::vector<vk_utils::queue_family> queue_families(
	const vk::raii::Instance& instance,
	const vk::raii::PhysicalDevice& phys_dev)
{
	std::vector props = phys_dev.getQueueFamilyProperties();

	std::vector<vk_utils::queue_family> res(props.size());

	int index = 0;
	std::ranges::transform(props, res.begin(), [&] (auto prop) -> vk_utils::queue_family {
		const bool present_supported = glfw::window::present_supported(instance, phys_dev, index);
		return vk_utils::queue_family(index++, prop, present_supported);
	});

	return res;
}


vk_utils::queue get_queue(
	const vk::raii::Device& dev,
	const vk_utils::queue_family& type,
	uint32_t index)
{
	return vk_utils::queue(
		type,
		dev.getQueue(
			type.index(),
			index));
}


}
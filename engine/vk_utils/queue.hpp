#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "queue_family.hpp"


namespace gorilla::vk_utils
{

class queue {
public:
	queue() = default;
	queue(queue_family type, vk::raii::Queue&& queue);

	[[nodiscard]] const vk::raii::Queue& vk_queue() const { return _handle; }
	[[nodiscard]] const queue_family& type() const { return _type; }
private:
	vk::raii::Queue _handle = nullptr;
	queue_family _type;
};

}

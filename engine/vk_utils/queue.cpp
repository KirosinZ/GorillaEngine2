#include "queue.hpp"

namespace gorilla::vk_utils {

queue::queue(
	queue_family type,
	vk::raii::Queue&& queue):
	_handle(std::move(queue)),
	_type(type)
{}


}
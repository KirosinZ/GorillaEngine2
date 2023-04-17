//
// Created by Kiril on 17.04.2023.
//

#ifndef DEEPLOM_BUFFER_HPP
#define DEEPLOM_BUFFER_HPP

#include "utils.hpp"

namespace vk_utils
{

class buffer
{
public:
	buffer() = delete;
	buffer(std::nullptr_t) {}

	inline buffer(const vk::raii::PhysicalDevice& phys_device, const vk::raii::Device& device, vk::DeviceSize size, vk::BufferUsageFlagBits usage, vk::MemoryPropertyFlags properties)
	{
		vk::BufferCreateInfo buffer_ci({}, size, usage);
		_buffer = device.createBuffer(buffer_ci);

		vk::MemoryRequirements memory_reqs = _buffer.getMemoryRequirements();

		vk::MemoryAllocateInfo allocate_info(memory_reqs.size, memory_type(phys_device, properties, memory_reqs.memoryTypeBits));
		_memory = device.allocateMemory(allocate_info);

		_buffer.bindMemory(*_memory, 0);
	}

	template<typename T>
	inline void set(T value, vk::DeviceSize size = sizeof(T))
	{
		void* data = _memory.mapMemory(0, size);
		std::memcpy(data, &value, size);
		_memory.unmapMemory();
	}

	inline const vk::Buffer& operator*() { return *_buffer; }

private:
	vk::raii::Buffer _buffer = nullptr;
	vk::raii::DeviceMemory _memory = nullptr;
};

} // vk_utils

#endif //DEEPLOM_BUFFER_HPP

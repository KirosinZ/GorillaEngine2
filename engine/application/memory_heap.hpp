#pragma once

#include <vulkan/vulkan.hpp>


namespace gorilla
{

class memory_heap
{
public:
	constexpr memory_heap() = default;

	memory_heap(uint32_t index, const vk::MemoryHeap& heap);

	bool operator==(const memory_heap& other) const;
	bool operator!=(const memory_heap& other) const;

public:
	[[nodiscard]] uint32_t index() const { return _index; }
	[[nodiscard]] uint64_t size() const { return _size; }
	[[nodiscard]] const vk::MemoryHeapFlags& flags() const { return _flags; }

	bool is_device_local() const;
private:
	uint32_t _index = std::numeric_limits<uint32_t>::max();
	vk::MemoryHeapFlags _flags;
	uint64_t _size = 0;
};

}
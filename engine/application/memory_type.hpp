#pragma once

#include "memory_heap.hpp"


namespace gorilla
{

class memory_type
{
public:
	constexpr memory_type() = default;

	memory_type(uint32_t index, const memory_heap& heap, const vk::MemoryType& type);

	bool operator==(const memory_type& other) const;
	bool operator!=(const memory_type& other) const;
public:

	[[nodiscard]] const memory_heap& heap() const { return _parent_heap; }
	[[nodiscard]] uint32_t index() const { return _index; }
	[[nodiscard]] const vk::MemoryPropertyFlags& properties() const { return _flags; }

	[[nodiscard]] bool is_host_visible() const;
	[[nodiscard]] bool is_host_coherent() const;
	[[nodiscard]] bool is_host_consistent() const;
	[[nodiscard]] bool is_device_local() const;
private:
	memory_heap _parent_heap;
	uint32_t _index = std::numeric_limits<uint32_t>::max();
	vk::MemoryPropertyFlags _flags;
};

}
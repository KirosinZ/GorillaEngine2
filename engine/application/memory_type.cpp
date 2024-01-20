#include "memory_type.hpp"

#include <utilities/asserts.hpp>


namespace gorilla
{

memory_type::memory_type(uint32_t index, const memory_heap& heap, const vk::MemoryType& type) :
	_parent_heap(heap),
	_index(index),
	_flags(type.propertyFlags)
{
	asserts_eq(type.heapIndex, heap.index());
}

bool memory_type::is_host_visible() const
{
	return (_flags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible;
}

bool memory_type::is_host_coherent() const
{
	return (_flags & vk::MemoryPropertyFlagBits::eHostCoherent) == vk::MemoryPropertyFlagBits::eHostCoherent;
}

bool memory_type::is_host_consistent() const
{
	return is_host_visible() && is_host_coherent();
}

bool memory_type::is_device_local() const
{
	return (_flags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal;
}

bool memory_type::operator==(const memory_type& other) const
{
	return std::tie(_parent_heap, _index, _flags) == std::tie(other._parent_heap, other._index, other._flags);
}

bool memory_type::operator!=(const memory_type& other) const
{
	return !(*this == other);
}

}
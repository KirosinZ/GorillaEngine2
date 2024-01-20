#include "queue_family.hpp"


namespace gorilla::vk_utils
{

queue_family::queue_family(uint32_t index, const vk::QueueFamilyProperties& props, bool present_supported) :
	_index(index),
	_count(props.queueCount),
	_flags(queue_flags{ static_cast<uint32_t>(props.queueFlags) }),
	_timestamp_valid_bits(props.timestampValidBits),
	_min_image_granularity(props.minImageTransferGranularity)
{
	if (present_supported)
		_flags |= queue_flag_bits::present;
}

queue_family::queue_family(uint32_t index, const vk::QueueFamilyProperties2& props, bool present_supported) :
	queue_family(index, props.queueFamilyProperties, present_supported)
{}

bool queue_family::has_graphics() const
{
	return (_flags & queue_flag_bits::graphics) == queue_flag_bits::graphics;
}

bool queue_family::has_present() const
{
	return (_flags & queue_flag_bits::present) == queue_flag_bits::present;
}

bool queue_family::has_all_graphics() const
{
	return has_graphics() && has_present();
}

bool queue_family::has_transfer() const
{
	return (_flags & queue_flag_bits::transfer) == queue_flag_bits::transfer;
}

bool queue_family::has_compute() const
{
	return (_flags & queue_flag_bits::compute) == queue_flag_bits::compute;
}

bool queue_family::operator==(const queue_family& other) const
{
	return std::tie(_index, _count, _flags, _timestamp_valid_bits, _min_image_granularity)
		== std::tie(other._index, other._count, other._flags, other._timestamp_valid_bits, other._min_image_granularity);
}

bool queue_family::operator!=(const queue_family& other) const
{
	return !(*this == other);
}

}
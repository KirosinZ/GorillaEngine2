#pragma once

#include <vulkan/vulkan.hpp>


namespace gorilla::vk_utils
{

enum class queue_flag_bits : uint32_t
{
	graphics = static_cast<uint32_t>(vk::QueueFlagBits::eGraphics),
	compute = static_cast<uint32_t>(vk::QueueFlagBits::eCompute),
	transfer = static_cast<uint32_t>(vk::QueueFlagBits::eTransfer),
	sparse_binding = static_cast<uint32_t>(vk::QueueFlagBits::eSparseBinding),
	protect = static_cast<uint32_t>(vk::QueueFlagBits::eProtected),
	present = 1 << 15u,
};

inline queue_flag_bits operator |(const queue_flag_bits& first, const queue_flag_bits& second)
{
	return queue_flag_bits{ static_cast<uint32_t>(first) | static_cast<uint32_t>(second) };
}

using queue_flags = vk::Flags<queue_flag_bits>;


class queue_family
{
public:
	constexpr queue_family() = default;

	queue_family(uint32_t index, const vk::QueueFamilyProperties& props, bool present_supported = false);
	queue_family(uint32_t index, const vk::QueueFamilyProperties2& props, bool present_supported = false);

	bool operator==(const queue_family& other) const;
	bool operator!=(const queue_family& other) const;

public:
	[[nodiscard]] uint32_t index() const { return _index; }
	[[nodiscard]] uint32_t queue_count() const { return _count; }
	[[nodiscard]] const queue_flags& capabilities() const { return _flags; }

	[[nodiscard]] uint32_t timestamp_mask() const { return _timestamp_valid_bits; }
	[[nodiscard]] const vk::Extent3D& min_image_granularity() const { return _min_image_granularity; }

	[[nodiscard]] bool has_graphics() const;
	[[nodiscard]] bool has_present() const;
	[[nodiscard]] bool has_all_graphics() const;

	[[nodiscard]] bool has_transfer() const;
	[[nodiscard]] bool has_compute() const;
private:
	uint32_t _index = std::numeric_limits<uint32_t>::max();
	uint32_t _count = 0;
	queue_flags _flags;

	uint32_t _timestamp_valid_bits = 0;
	vk::Extent3D _min_image_granularity;
};

}
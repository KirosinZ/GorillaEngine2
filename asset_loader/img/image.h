//
// Created by Kiril on 21.02.2023.
//

#ifndef DEEPLOM_IMAGE_H
#define DEEPLOM_IMAGE_H

#include <vector>
#include <string>

namespace gorilla::img
{

class image
{
public:
	static image load_image(const std::string& filename);

	inline const std::vector<uint8_t>& data() const { return _bytes; }

	inline int width() const { return _width; }
	inline int height() const { return _height; }
	inline int channels() const { return _channels; }

	friend bool operator==(const image& l, const image& r) noexcept;
	inline friend bool operator!=(const image& l, const image& r) noexcept { return !(l == r); };
private:

	std::vector<uint8_t> _bytes;
	int _width = 0;
	int _height = 0;
	int _channels = 0;
};

static_assert(std::is_nothrow_default_constructible_v<image>);
static_assert(std::is_copy_constructible_v<image>);
static_assert(std::is_copy_assignable_v<image>);
static_assert(std::is_nothrow_move_constructible_v<image>);
static_assert(std::is_nothrow_move_assignable_v<image>);
static_assert(std::is_nothrow_destructible_v<image>);

static_assert(std::is_nothrow_swappable_v<image>);

} // gorilla::img

#endif //DEEPLOM_IMAGE_H

//
// Created by Kiril on 21.02.2023.
//

#include <stdexcept>

#include "image.h"

#include "stb/stb_image.h"

namespace gorilla::img
{

image image::load_image(const std::string& filename)
{
	image res;
	stbi_uc* bytes = stbi_load(filename.c_str(), &res._width, &res._height, &res._channels, STBI_rgb_alpha);
	if (bytes == nullptr)
		throw std::runtime_error(stbi_failure_reason());

	res._bytes = std::vector<uint8_t>(bytes, bytes + res._height * res._width * res._channels);
	stbi_image_free(bytes);

	return res;
}

bool operator==(const image& l, const image& r) noexcept
{
	return l._bytes == r._bytes
		&& l._width == r._width
		&& l._height == r._height
		&& l._channels == r._channels;
}

} // gorilla::img
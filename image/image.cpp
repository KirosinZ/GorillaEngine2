#include "image.hpp"

#include <../external/stb/stb_image.h>
#include <../external/stb/stb_image_write.h>

#include <utilities/asserts.hpp>


namespace gorilla::asset
{

image::image(
	std::vector<uint8_t> bytes,
	uint32_t width,
	uint32_t height) :
	_bytes(std::move(bytes)),
	_width(width),
	_height(height)
{
	asserts(_bytes.size() == _width * _height * 4, "size mismatch");
}


std::optional<image> image::load_image(const std::filesystem::path& file)
{
	int w, h, c;
	stbi_uc* bytes = stbi_load(file.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
	if (bytes == nullptr)
		platform::terminate(stbi_failure_reason());

	auto tmp = std::vector(bytes, bytes + h * w * 4);
	stbi_image_free(bytes);

	return image(std::move(tmp), w, h);
}

void image::save_image(
	const std::filesystem::path& file) const
{
	const std::string ext = file.extension().string();

	// if (ext == ".bmp")
	// 	stbi_write_bmp(file.string().c_str(), _width, _height, 0, _bytes.data());
	// else if (ext == ".tga")
	// 	stbi_write_tga(file.string().c_str(), _width, _height, 0, _bytes.data());
	// else if (ext == ".jpg")
	// 	stbi_write_jpg(file.string().c_str(), _width, _height, 0, _bytes.data(), 0);
	// else if (ext == ".png")
	// 	stbi_write_png(file.string().c_str(), _width, _height, 0, _bytes.data(), 4);
}


bool operator==(const image& l, const image& r) noexcept
{
	return l._bytes == r._bytes
		&& l._width == r._width
		&& l._height == r._height;
}

} // gorilla::asset
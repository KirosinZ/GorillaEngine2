#pragma once

#include <filesystem>
#include <vector>


namespace gorilla::asset
{

class image
{
public:
	image() = default;
	image(std::vector<uint8_t> bytes, uint32_t width, uint32_t height);

	static std::optional<image> load_image(const std::filesystem::path& file);
	void save_image(const std::filesystem::path& file) const;

	[[nodiscard]] const std::vector<uint8_t>& data() const { return _bytes; }

	[[nodiscard]] size_t byte_size() const { return _bytes.size(); }
	[[nodiscard]] const uint8_t* raw_data() const { return _bytes.data(); }

	[[nodiscard]] uint32_t width() const { return _width; }
	[[nodiscard]] uint32_t height() const { return _height; }
	[[nodiscard]] size_t size() const { return _width * _height; }

	friend bool operator==(const image& l, const image& r) noexcept;
	friend bool operator!=(const image& l, const image& r) noexcept { return !(l == r); };
private:

	std::vector<uint8_t> _bytes;
	uint32_t _width = 0;
	uint32_t _height = 0;
};

} // gorilla::asset

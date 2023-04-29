//
// Created by Kiril on 12.04.2023.
//

#ifndef DEEPLOM_TEXTURE_HPP
#define DEEPLOM_TEXTURE_H

#include <vulkan/vulkan_raii.hpp>

namespace gorilla::engine
{

struct texture
{
	uint32_t mip_levels = 0u;
	vk::raii::Image image = nullptr;
	vk::raii::DeviceMemory mem = nullptr;
	vk::raii::ImageView image_view = nullptr;
	vk::raii::Sampler sampler = nullptr;
};

} // gorilla::engine

#endif //DEEPLOM_TEXTURE_HPP

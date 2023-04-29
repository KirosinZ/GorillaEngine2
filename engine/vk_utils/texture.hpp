//
// Created by Kiril on 24.04.2023.
//

#ifndef DEEPLOM_TEXTURE_HPP
#define DEEPLOM_TEXTURE_HPP

#include <vulkan/vulkan_raii.hpp>

namespace vk_utils
{

class texture
{
public:

private:
	uint32_t m_mip_levels = 0u;
	vk::raii::Image m_image = nullptr;
	vk::raii::DeviceMemory m_memory = nullptr;
	vk::raii::ImageView m_view = nullptr;
};

} // vk_utils

#endif //DEEPLOM_TEXTURE_HPP

//
// Created by Kiril on 12.04.2023.
//

#ifndef DEEPLOM_GEOMBUFFER_H
#define DEEPLOM_GEOMBUFFER_H

#include <vulkan/vulkan_raii.hpp>

namespace gorilla::engine
{

struct geom_buffer
{
	vk::raii::Buffer vertex_buffer = nullptr;
	vk::raii::DeviceMemory vertex_buffer_memory = nullptr;

	vk::raii::Buffer index_buffer = nullptr;
	vk::raii::DeviceMemory index_buffer_memory = nullptr;
};

} // gorilla::engine

#endif //DEEPLOM_GEOMBUFFER_H

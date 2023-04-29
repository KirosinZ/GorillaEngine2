//
// Created by Kiril on 22.04.2023.
//

#include "environment.hpp"

namespace vk_utils
{

environment::environment(
		vk::raii::Instance&& t_instance,
		vk::raii::PhysicalDevice&& t_phys_device,
		vk::raii::Device&& t_device,
		int t_graphics_queue_index,
		vk::raii::Queue&& t_graphics_queue,
		std::vector<vk::raii::SurfaceKHR>&& t_surfaces,
		int t_present_queue_index,
		vk::raii::Queue&& t_present_queue)
		: m_instance(std::move(t_instance)),
		  m_phys_device(std::move(t_phys_device)),
		  m_device(std::move(t_device)),
		  m_graphics_queue_index(t_graphics_queue_index),
		  m_graphics_queue(std::move(t_graphics_queue)),
		  m_surfaces(std::move(t_surfaces)),
		  m_present_queue_index(t_present_queue_index),
		  m_present_queue(std::move(t_present_queue))
{}

} // vk_utils
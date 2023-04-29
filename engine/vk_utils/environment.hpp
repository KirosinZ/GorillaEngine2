//
// Created by Kiril on 22.04.2023.
//

#ifndef DEEPLOM_ENVIRONMENT_HPP
#define DEEPLOM_ENVIRONMENT_HPP

#include <vulkan/vulkan_raii.hpp>
#include <map>


namespace vk_utils
{

class environment
{
public:

	environment() = default;
	environment(
			vk::raii::Instance&& t_instance,
			vk::raii::PhysicalDevice&& t_phys_device,
			vk::raii::Device&& t_device,
			int t_graphics_queue_index,
			vk::raii::Queue&& t_graphics_queue,
			std::vector<vk::raii::SurfaceKHR>&& t_surfaces = {},
			int t_present_queue_index = -1,
			vk::raii::Queue&& t_present_queue = nullptr);

	static const vk::raii::Context& context() { return m_context; }

	const vk::raii::Instance& instance() const { return m_instance; }
	const vk::raii::PhysicalDevice& phys_device() const { return m_phys_device; }
	const vk::raii::Device& device() const { return m_device; }

	const bool has_surfaces() const { return m_surfaces.size() != 0; }
	const size_t n_surfaces() const { return m_surfaces.size(); }
	const std::vector<vk::raii::SurfaceKHR>& surfaces() const { return m_surfaces; }
	const vk::raii::SurfaceKHR& surface(size_t index) const { return m_surfaces[index]; }

	int graphics_queue_index() const { return m_graphics_queue_index; }
	const vk::raii::Queue& graphics_queue() const { return m_graphics_queue; }

	int present_queue_index() const { return m_present_queue_index; }
	const vk::raii::Queue& present_queue() const { return m_present_queue; }
private:
	inline static vk::raii::Context m_context{};

	vk::raii::Instance m_instance = nullptr;
	vk::raii::PhysicalDevice m_phys_device = nullptr;
	std::vector<vk::raii::SurfaceKHR> m_surfaces;
	vk::raii::Device m_device = nullptr;

	int m_graphics_queue_index{};
	vk::raii::Queue m_graphics_queue = nullptr;

	int m_present_queue_index{};
	vk::raii::Queue m_present_queue = nullptr;
};

} // vk_utils

#endif //DEEPLOM_ENVIRONMENT_HPP

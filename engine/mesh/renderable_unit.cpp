//
// Created by Kiril on 16.03.2023.
//

#include "renderable_unit.hpp"

#include <vulkan/vulkan_raii.hpp>

namespace gorilla::engine
{

vk::VertexInputBindingDescription vk_input_binding_description_mesh(uint32_t binding)
{
	vk::VertexInputBindingDescription vibd(
			binding,
			sizeof(gorilla::geom::triangulated_mesh::vertex),
			vk::VertexInputRate::eVertex);

	return vibd;
}

std::vector<vk::VertexInputAttributeDescription> vk_input_attribute_descriptions_mesh(uint32_t binding)
{
	std::vector<vk::VertexInputAttributeDescription> viads;
	viads.emplace_back(0, binding, vk::Format::eR32G32B32Sfloat, offsetof(gorilla::geom::triangulated_mesh::vertex, pos));
	viads.emplace_back(1, binding, vk::Format::eR32G32B32Sfloat, offsetof(gorilla::geom::triangulated_mesh::vertex, tex));
	viads.emplace_back(2, binding, vk::Format::eR32G32B32Sfloat, offsetof(gorilla::geom::triangulated_mesh::vertex, norm));

	return viads;
}

vk::VertexInputBindingDescription vk_input_binding_description_wireframe(uint32_t binding)
{
	vk::VertexInputBindingDescription vibd(
			binding,
			sizeof(glm::vec3),
			vk::VertexInputRate::eVertex);

	return vibd;
}

std::vector<vk::VertexInputAttributeDescription> vk_input_attribute_descriptions_wireframe(uint32_t binding)
{
	std::vector<vk::VertexInputAttributeDescription> viads;
	viads.emplace_back(0, binding, vk::Format::eR32G32B32Sfloat, 0);

	return viads;
}

} // gorilla::engine
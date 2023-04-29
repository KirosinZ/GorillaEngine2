//
// Created by Kiril on 16.03.2023.
//

#ifndef DEEPLOM_RENDERABLE_UNIT_HPP
#define DEEPLOM_RENDERABLE_UNIT_HPP

#include <vector>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <triangulated_mesh/triangulatedmesh.h>
#include <wireframe/wireframe.h>
#include <img/image.h>
#include <stdexcept>
#include "mesh.hpp"

namespace gorilla::engine
{

class renderable_unit
{
public:
	using mesh_t = mesh;
	using wireframe_t = geom::wireframe;

	renderable_unit() = default;
	renderable_unit(mesh_t mesh, vk::raii::Pipeline& pipeline, glm::mat4 model = glm::mat4(1.0f))
	: _mesh(std::move(mesh)), _pipeline(&pipeline), _model(model) {}

	inline const mesh_t& mesh() const { return _mesh; }

	inline const glm::mat4& model() const { return _model; }
	inline glm::mat4& model() { return _model; }

	inline const vk::raii::Pipeline& pipeline() const { return *_pipeline; }

	inline bool has_wireframe() const { return _wireframe != wireframe_t{}; }
	inline const wireframe_t& wireframe() const { return _wireframe; }
	inline wireframe_t& wireframe() { return _wireframe; }

	inline bool has_texture() const { return _texture != nullptr; }
	inline const asset::image& texture() const
	{
		if (_texture == nullptr)
			throw std::runtime_error("texture is not present");

		return *_texture;
	}
	inline void set_texture(asset::image& texture) { _texture = &texture; }

	inline bool has_normal_map() const { return _normal_map != nullptr; }
	inline const asset::image& normal_map() const
	{
		if (_normal_map == nullptr)
			throw std::runtime_error("normal map is not present");

		return *_normal_map;
	}
	inline void set_normal_map(asset::image& normal_map) { _normal_map = &normal_map; }
private:
	mesh_t _mesh;
	glm::mat4 _model = glm::mat4(1.0f);

	wireframe_t _wireframe{};

	asset::image* _texture = nullptr;
	asset::image* _normal_map = nullptr;
	vk::raii::Pipeline* _pipeline = nullptr;
};

static_assert(std::is_default_constructible_v<renderable_unit>);
static_assert(std::is_copy_constructible_v<renderable_unit>);
static_assert(std::is_copy_assignable_v<renderable_unit>);
static_assert(std::is_nothrow_move_constructible_v<renderable_unit>);
static_assert(std::is_nothrow_move_assignable_v<renderable_unit>);
static_assert(std::is_nothrow_destructible_v<renderable_unit>);

static_assert(std::is_nothrow_swappable_v<renderable_unit>);

} // gorilla::engine

#endif //DEEPLOM_RENDERABLE_UNIT_HPP

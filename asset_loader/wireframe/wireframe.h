//
// Created by Kiril on 31.03.2023.
//

#ifndef DEEPLOM_WIREFRAME_H
#define DEEPLOM_WIREFRAME_H

#include <vector>
#include <glm/glm.hpp>

#include <obj/obj.h>
#include <triangulated_mesh/triangulatedmesh.h>

namespace gorilla::geom
{

class wireframe
{
public:
	static wireframe from_obj(const obj& obj);
	static wireframe from_triangulated_mesh(const triangulated_mesh& mesh);

	inline const std::vector<glm::vec3>& points() const { return _points; }
	inline const std::vector<uint32_t>& indices() const { return _indices; }

	friend bool operator==(const wireframe& l, const wireframe& r) noexcept;
	inline friend bool operator!=(const wireframe& l, const wireframe& r) noexcept { return !(l == r); }
private:
	std::vector<glm::vec3> _points;
	std::vector<uint32_t> _indices;
};

static_assert(std::is_nothrow_default_constructible_v<wireframe>);
static_assert(std::is_copy_constructible_v<wireframe>);
static_assert(std::is_copy_assignable_v<wireframe>);
static_assert(std::is_nothrow_move_constructible_v<wireframe>);
static_assert(std::is_nothrow_move_assignable_v<wireframe>);
static_assert(std::is_nothrow_destructible_v<wireframe>);

static_assert(std::is_nothrow_swappable_v<wireframe>);

} // gorilla::geom

#endif //DEEPLOM_WIREFRAME_H

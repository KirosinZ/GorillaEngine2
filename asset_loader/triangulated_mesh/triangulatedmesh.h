//
// Created by Kiril on 28.03.2023.
//

#ifndef DEEPLOM_TRIANGULATEDMESH_H
#define DEEPLOM_TRIANGULATEDMESH_H

#include <glm/glm.hpp>

#include <asset_loader/obj/obj.h>

namespace gorilla::geom
{

class triangulated_mesh
{
public:
	struct vertex
	{
		glm::vec3 pos;
		glm::vec3 tex;
		glm::vec3 norm;
	};

	static triangulated_mesh from_obj(const obj& obj);

	inline const std::vector<vertex>& vertices() const { return _vertices; }
	inline std::vector<vertex>& vertices() { return _vertices; }

	inline const std::vector<uint32_t>& indices() const { return _indices; }

	friend bool operator==(const triangulated_mesh& l, const triangulated_mesh& r) noexcept;
	inline friend bool operator!=(const triangulated_mesh& l, const triangulated_mesh& r) noexcept { return !(l == r); }
private:
	std::vector<vertex> _vertices;
	std::vector<uint32_t> _indices;
};

static_assert(std::is_nothrow_default_constructible_v<triangulated_mesh>);
static_assert(std::is_copy_constructible_v<triangulated_mesh>);
static_assert(std::is_copy_assignable_v<triangulated_mesh>);
static_assert(std::is_nothrow_move_constructible_v<triangulated_mesh>);
static_assert(std::is_nothrow_move_assignable_v<triangulated_mesh>);
static_assert(std::is_nothrow_destructible_v<triangulated_mesh>);

static_assert(std::is_nothrow_swappable_v<triangulated_mesh>);

} // gorilla::geom

#endif //DEEPLOM_TRIANGULATEDMESH_H

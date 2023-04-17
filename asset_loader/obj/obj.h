//
// Created by Kiril on 19.02.2023.
//

#ifndef DEEPLOM_OBJ_H
#define DEEPLOM_OBJ_H

#include <vector>
#include <string>
#include <map>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

namespace gorilla::geom
{

class obj
{
public:
	struct vertex_info_t
	{
		std::vector<glm::vec4> v;
		std::vector<glm::vec3> vt;
		std::vector<glm::vec3> vn;
	};

	struct index_triplet
	{
		int32_t vi = -1;
		int32_t vti = -1;
		int32_t vni = -1;

		friend bool operator==(const obj::index_triplet& l, const obj::index_triplet& r) noexcept;
		inline friend bool operator!=(const obj::index_triplet& l, const obj::index_triplet& r) noexcept { return !(l == r); };
	};

	struct face_info_t
	{
		std::vector<index_triplet> face_indices;
		std::vector<uint32_t> face_offsets;
	};

	struct grouping_info_t
	{
		std::vector<uint32_t> g_ind;
		std::vector<uint32_t> o_ind;
		std::vector<uint32_t> mtl_ind;

		std::vector<std::string> g_names;
		std::vector<std::string> o_names;
		std::vector<std::string> mtl_filenames;
	};

	struct misc_info_t
	{
		bool uniform_face_side_count = false;
		uint32_t face_side_count = 0;

		bool triangulated = false;
	};

	obj() = default;
	obj(vertex_info_t vertices, face_info_t faces, grouping_info_t groupings = {});

	static obj load_obj(const std::string& filename);


	inline const vertex_info_t& vertex_info() const { return _vertices; }

	inline const std::vector<glm::vec4>& vertices() const { return _vertices.v; }
	inline size_t n_vertices() const { return _vertices.v.size(); }

	inline const std::vector<glm::vec3>& texcoords() const { return _vertices.vt; }
	inline size_t n_texcoords() const { return _vertices.vt.size(); }
	inline bool has_texcoords() const { return n_texcoords() > 0; }

	inline const std::vector<glm::vec3>& normals() const { return _vertices.vn; }
	inline size_t n_normals() const { return _vertices.vn.size(); }
	inline bool has_normals() const { return n_normals() > 0; }


	inline const face_info_t& face_info() const { return _faces; }
	inline const std::vector<index_triplet>& face_indices() const { return _faces.face_indices; }
	inline size_t n_face_indices() const { return _faces.face_indices.size(); }

	inline const std::vector<uint32_t>& face_offsets() const { return _faces.face_offsets; }
	inline size_t n_faces() const { return std::max(_faces.face_offsets.size() - 1, 0ull); }


	inline const grouping_info_t& grouping_info() const { return _groupings; }

	inline const std::vector<uint32_t>& group_indices() const { return _groupings.g_ind; }
	inline const std::vector<std::string>& group_names() const { return _groupings.g_names; }
	inline size_t n_groups() const { return _groupings.g_names.size(); }
	inline bool has_groups() const { return n_groups() > 0; }

	inline const std::vector<uint32_t>& object_indices() const { return _groupings.o_ind; }
	inline const std::vector<std::string>& object_names() const { return _groupings.o_names; }
	inline size_t n_objects() const { return _groupings.o_names.size(); }
	inline bool has_objects() const { return n_objects() > 0; }

	inline const std::vector<uint32_t>& material_indices() const { return _groupings.mtl_ind; }


	inline const misc_info_t& misc_info() const { return _misc; }

	inline bool is_triangulated() const { return _misc.triangulated; }


	friend bool operator==(const obj& l, const obj& r) noexcept;
	inline friend bool operator!=(const obj& l, const obj& r) noexcept { return !(l == r); };
private:
	vertex_info_t _vertices;
	face_info_t _faces;
	grouping_info_t _groupings;
	misc_info_t _misc;

	std::vector<uint32_t> _offsets;
};

const obj nullobj{};

static_assert(std::is_nothrow_default_constructible_v<obj>);
static_assert(std::is_copy_constructible_v<obj>);
static_assert(std::is_copy_assignable_v<obj>);
static_assert(std::is_nothrow_move_constructible_v<obj>);
static_assert(std::is_nothrow_move_assignable_v<obj>);
static_assert(std::is_nothrow_destructible_v<obj>);

static_assert(std::is_nothrow_swappable_v<obj>);

obj triangulate_obj(const obj& obj);
std::vector<glm::vec3> calculate_normals(const std::vector<glm::vec4>& positions, const std::vector<obj::index_triplet>& triangle_indices);
std::vector<glm::mat3> calculate_normals_tangents_bitangents(const std::vector<glm::vec4>& positions, const std::vector<glm::vec3>& texcoords, const std::vector<obj::index_triplet>& triangle_indices);

template <typename T>
inline std::vector<T> triangulate(const std::vector<T>& flatdata, const std::vector<uint32_t>& offsets)
{
	std::vector<T> res;
	const int n_faces = offsets.size() - 1;

	for (int i = 0; i < n_faces; i++)
	{
		const int offset = offsets[i];
		const int count = offsets[i + 1] - offset;

		for (int j = 0; j <= count / 2; j++)
		{
			int index = offset + (-(j + 1) / 2 + count) % count;
			res.push_back(flatdata[index]);

			index = offset + (j / 2 + 1 + count) % count;
			res.push_back(flatdata[index]);

			index = (j + 1) / 2 + 1;
			index = offset + ((j % 2 == 0 ? -index : index) + count) % count;
			res.push_back(flatdata[index]);
		}
	}

	return res;
}

} // gorilla::obj

#endif //DEEPLOM_OBJ_H

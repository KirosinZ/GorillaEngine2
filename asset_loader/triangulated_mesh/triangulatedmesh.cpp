//
// Created by Kiril on 28.03.2023.
//

#include <map>

#include "triangulatedmesh.h"

std::vector<uint32_t> triangulate_polygon(std::vector<uint32_t>::const_iterator first, std::vector<uint32_t>::const_iterator last)
{
	const std::size_t size = last - first;
	if (size == 3)
		return {first, last};

	const auto f = [&](uint32_t i)
	{
		const int n = -(int(i) + 1) / 2;
		return *(n < 0 ? last + n : first + n);
	};

	const auto s = [&](uint32_t i)
	{
		const int n = i / 2 + 1;
		return *(first + n);
	};

	const auto t = [&](uint32_t i)
	{
		const uint32_t r = 1 + (i + 1) / 2;
		int n = (i % 2 == 0 ? -r : r);
		return *(n < 0 ? last + n : first + n);
	};

	std::vector<uint32_t> res;
	for (int i = 0; i < (size + 1) / 2; i++)
	{
		res.push_back(f(i));
		res.push_back(s(i));
		res.push_back(t(i));
	}

	return res;
}

namespace gorilla::geom
{

triangulated_mesh triangulated_mesh::from_obj(const obj& obj)
{
//	using triplet = std::tuple<int32_t, int32_t, int32_t>;
//	std::map<triplet, uint32_t> map;
//
	triangulated_mesh res;
//	triangulated_mesh::vertex vertex{};
//
//	for (int i = 0; i < obj.face_offsets().size() - 1; i++)
//	{
//		const int offset = obj.face_offsets()[i];
//		const int count = obj.face_offsets()[i + 1] - offset;
//
//		std::vector<uint32_t> v_i = triangulate_polygon(obj.vertex_indices().begin() + offset, obj.vertex_indices().begin() + offset + count);
//		std::vector<uint32_t> vt_i;
//		if (!obj.texcoord_indices().empty())
//			vt_i = triangulate_polygon(obj.texcoord_indices().begin() + offset, obj.texcoord_indices().begin() + offset + count);
//		std::vector<uint32_t> vn_i;
//		if (!obj.normal_indices().empty())
//			vn_i = triangulate_polygon(obj.normal_indices().begin() + offset, obj.normal_indices().begin() + offset + count);
//
//		for (int index = 0; index < v_i.size(); index++)
//		{
//			const uint32_t vi = v_i[index];
//			const uint32_t vti = vt_i.empty() ? 0 : vt_i[index];
//			const uint32_t vni = vn_i.empty() ? 0 : vn_i[index];
//			const triplet triplet = std::make_tuple(vi, vti, vni);
//
//			vertex.pos = obj.vertices()[vi];
//			vertex.tex = vt_i.empty() ? glm::vec3(0.0f) : obj.texcoords()[vti];
//			vertex.norm = vn_i.empty() ? glm::vec3(0.0f) : obj.normals()[vni];
//
//			if (map.find(triplet) == map.end())
//			{
//				map.insert_or_assign(triplet, res._vertices.size());
//				res._vertices.push_back(vertex);
//			}
//
//			res._indices.push_back(map.at(triplet));
//		}
//	}
//
	return res;
}

bool operator==(const triangulated_mesh& l, const triangulated_mesh& r) noexcept
{
	// TODO: Implementation needed
	return false;
}

} // gorilla::geom
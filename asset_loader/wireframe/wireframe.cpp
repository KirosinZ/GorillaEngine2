//
// Created by Kiril on 31.03.2023.
//

#include "wireframe.h"

#include <set>
#include <stdexcept>


namespace gorilla::geom
{

wireframe wireframe::from_obj(const gorilla::geom::obj& obj)
{
	wireframe res;
//	res._points = obj.vertices();
//
//	std::set<std::pair<uint32_t, uint32_t>> lines;
//
//	for (int i = 0; i < obj.face_offsets().size() - 1; i++)
//	{
//		const uint32_t offset = obj.face_offsets()[i];
//		const uint32_t count = obj.face_offsets()[i + 1] - offset;
//		for (int j = 0; j < count; j++)
//		{
//			const uint32_t first = obj.vertex_indices()[offset + j];
//			const uint32_t second = obj.vertex_indices()[offset + (j + 1) % count];
//			std::pair<uint32_t, uint32_t> line;
//			if (first < second)
//			{
//				line.first = first;
//				line.second = second;
//			}
//			else if (first > second)
//			{
//				line.first = second;
//				line.second = first;
//			}
//			else
//				throw std::runtime_error("Identical vertices in a polygon");
//
//			if (lines.insert(line).second)
//			{
//				res._indices.push_back(line.first);
//				res._indices.push_back(line.second);
//			}
//		}
//	}
//
	return res;
}

wireframe wireframe::from_triangulated_mesh(const gorilla::geom::triangulated_mesh& mesh)
{
	wireframe res;
	res._points.resize(mesh.vertices().size());
	for (int i = 0; i < res._points.size(); i++)
	{
		res._points[i] = mesh.vertices()[i].pos;
	}

	std::set<std::pair<uint32_t, uint32_t>> lines;

	for (int i = 0; i < mesh.indices().size() / 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			const uint32_t first = mesh.indices()[3 * i + j];
			const uint32_t second = mesh.indices()[3 * i + (j + 1) % 3];
			std::pair<uint32_t, uint32_t> line;
			if (first < second)
			{
				line.first = first;
				line.second = second;
			}
			else if (first > second)
			{
				line.first = second;
				line.second = first;
			}
			else
				throw std::runtime_error("Identical vertices in a polygon");

			if (lines.insert(line).second)
			{
				res._indices.push_back(line.first);
				res._indices.push_back(line.second);
			}
		}
	}

	return res;
}

bool operator==(const wireframe& l, const wireframe& r) noexcept
{
	return l._points == r._points
		&& l._indices == r._indices;
}

} // gorilla::geom
//
// Created by Kiril on 05.04.2023.
//

#ifndef DEEPLOM_MESH_HPP
#define DEEPLOM_MESH_HPP

#include <glm/glm.hpp>
#include <glm/mat2x3.hpp>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <set>
#include "obj/obj.h"


namespace gorilla
{

struct mesh
{
	struct vertex
	{
		glm::vec3 position;
		glm::vec3 texcoord;
		glm::vec3 normal;
		glm::vec4 tangent;
	};

	std::vector<vertex> vertices;
	std::vector<uint32_t> indices;

	static mesh from_obj(const asset::obj& obj);
	static void calculate_tangent_space(mesh& mesh);
};

template <typename T>
vk::VertexInputBindingDescription vk_vertex_input_binding_description(uint32_t binding) = delete;

template <typename T>
std::vector<vk::VertexInputAttributeDescription> vk_vertex_input_attribute_description(uint32_t binding) = delete;

template <>
inline vk::VertexInputBindingDescription vk_vertex_input_binding_description<mesh>(uint32_t binding)
{
	vk::VertexInputBindingDescription res(
			binding,
			sizeof(mesh::vertex),
			vk::VertexInputRate::eVertex);

	return res;
}

template <>
inline std::vector<vk::VertexInputAttributeDescription> vk_vertex_input_attribute_description<mesh>(uint32_t binding)
{
	std::vector<vk::VertexInputAttributeDescription> res;
	res.emplace_back(0, binding, vk::Format::eR32G32B32Sfloat, offsetof(mesh::vertex, position));
	res.emplace_back(1, binding, vk::Format::eR32G32B32Sfloat, offsetof(mesh::vertex, texcoord));
	res.emplace_back(2, binding, vk::Format::eR32G32B32Sfloat, offsetof(mesh::vertex, normal));
	res.emplace_back(3, binding, vk::Format::eR32G32B32A32Sfloat, offsetof(mesh::vertex, tangent));

	return res;
}

inline mesh mesh::from_obj(const asset::obj& obj)
{
	const asset::obj& tri_obj = obj.is_triangulated() ? obj : asset::triangulate_obj(obj);

	mesh res{};

	std::map<std::pair<int32_t, int32_t>, int32_t> vertices;

	std::vector<std::set<int>> vertex_to_triangle(tri_obj.n_vertices());

	for (const asset::obj::index_triplet& t : tri_obj.face_indices())
	{
		std::pair<int32_t, int32_t> key = { t.vi, t.vti };

		const auto& thing = vertices.find(key);

		if (thing == vertices.end())
		{
			mesh::vertex v{};
			v.position = tri_obj.vertices()[t.vi];
			v.texcoord = tri_obj.texcoords()[t.vti];
			v.normal = tri_obj.normals()[t.vni];
//			v.texcoord.y = 1.0f - v.texcoord.y;

			vertices.insert_or_assign(key, res.vertices.size());
			res.vertices.push_back(v);
		}
		res.indices.push_back(vertices.at(key));
		vertex_to_triangle[t.vi].insert(vertices.at(key));
	}

	std::vector<std::vector<int>> triangle_to_vertex(obj.n_face_indices());
	for (int i = 0; i < vertex_to_triangle.size(); i++)
	{
		for (auto iter = vertex_to_triangle[i].begin(); iter != vertex_to_triangle[i].end(); iter++)
		{
			for (auto inter = vertex_to_triangle[i].begin(); inter != vertex_to_triangle[i].end(); inter++)
			{
				triangle_to_vertex[*iter].push_back(*inter);
			}
		}
	}
	calculate_tangent_space(res);

	return res;
}

inline void mesh::calculate_tangent_space(mesh& mesh)
{
	const int n_triangles = mesh.indices.size() / 3;
	const int n_vertices = mesh.vertices.size();
	std::vector<float> weights(n_vertices);
	std::vector<glm::vec3> tangents(n_vertices);
	std::vector<glm::vec3> bitangents(n_vertices);
	for (int i = 0; i < n_triangles; i++)
	{
		mesh::vertex& v0 = mesh.vertices[mesh.indices[3 * i + 0]];
		mesh::vertex& v1 = mesh.vertices[mesh.indices[3 * i + 1]];
		mesh::vertex& v2 = mesh.vertices[mesh.indices[3 * i + 2]];

		float& w0 = weights[mesh.indices[3 * i + 0]];
		float& w1 = weights[mesh.indices[3 * i + 1]];
		float& w2 = weights[mesh.indices[3 * i + 2]];

		glm::vec3& t0 = tangents[mesh.indices[3 * i + 0]];
		glm::vec3& t1 = tangents[mesh.indices[3 * i + 1]];
		glm::vec3& t2 = tangents[mesh.indices[3 * i + 2]];

		glm::vec3& b0 = bitangents[mesh.indices[3 * i + 0]];
		glm::vec3& b1 = bitangents[mesh.indices[3 * i + 1]];
		glm::vec3& b2 = bitangents[mesh.indices[3 * i + 2]];

		const glm::vec3 q1 = v1.position - v0.position;
		const glm::vec3 q2 = v2.position - v0.position;

		const glm::vec3 n = glm::cross(q1, q2);
		float w = glm::length(n);
		v0.normal += n; w0 += w;
		v1.normal += n; w1 += w;
		v2.normal += n; w2 += w;

		const glm::vec2 uv1 = v1.texcoord - v0.texcoord;
		const glm::vec2 uv2 = v2.texcoord - v0.texcoord;

		const glm::mat2x3 TB = glm::mat2x3(q1, q2) * glm::inverse(glm::mat2(uv1, uv2));
		glm::vec3 T = TB[0];
		glm::vec3 B = TB[1];

		t0 += T;
		t1 += T;
		t2 += T;

		b0 += B;
		b1 += B;
		b2 += B;
	}

	for (int i = 0; i < n_vertices; i++)
	{
		mesh::vertex& v = mesh.vertices[i];
		glm::vec3& t = tangents[i];
		glm::vec3& b = bitangents[i];

		v.normal /= weights[i];
		t /= weights[i];
		b /= weights[i];

		t -= glm::dot(v.normal, t) * v.normal;
		b -= glm::dot(v.normal, b) * v.normal + glm::dot(t, b) * t / glm::dot(t, t);

		v.normal = glm::normalize(v.normal);
		t = glm::normalize(t);
		b = glm::normalize(b);

		v.tangent = glm::vec4(t, glm::determinant(glm::mat3(t, b, v.normal)));
		int x = 0;
	}
}

inline void calculate_tangent_space(mesh& mesh, const std::vector<std::vector<int>>& triangle_to_vertex)
{
	const int n_triangles = mesh.indices.size() / 3;
	const int n_vertices = mesh.vertices.size();
	std::vector<float> weights(n_vertices);
	std::vector<glm::vec3> tangents(n_vertices);
	std::vector<glm::vec3> bitangents(n_vertices);
	for (int i = 0; i < n_triangles; i++)
	{
		std::vector<int> tis0 = triangle_to_vertex[mesh.indices[3 * i + 0]];
		std::vector<int> tis1 = triangle_to_vertex[mesh.indices[3 * i + 1]];
		std::vector<int> tis2 = triangle_to_vertex[mesh.indices[3 * i + 2]];

		mesh::vertex v0 = mesh.vertices[mesh.indices[3 * i + 0]];
		mesh::vertex v1 = mesh.vertices[mesh.indices[3 * i + 1]];
		mesh::vertex v2 = mesh.vertices[mesh.indices[3 * i + 2]];

		const glm::vec3 q1 = v1.position - v0.position;
		const glm::vec3 q2 = v2.position - v0.position;

		const glm::vec3 n = glm::cross(q1, q2);
		/*v0.normal += n;*/ float w = glm::length(n);
		/*v1.normal += n;*/
		/*v2.normal += n;*/

		const glm::vec2 uv1 = v1.texcoord - v0.texcoord;
		const glm::vec2 uv2 = v2.texcoord - v0.texcoord;

		const glm::mat2x3 TB = glm::mat2x3(q1, q2) * glm::inverse(glm::mat2(uv1, uv2));
		glm::vec3 T = w * TB[0];
		glm::vec3 B = w * TB[1];

		for (int j : tis0)
		{
			tangents[j] += T;
			bitangents[j] += B;
			weights[j] += w;
		}
		for (int j : tis1)
		{
			tangents[j] += T;
			bitangents[j] += B;
			weights[j] += w;
		}
		for (int j : tis2)
		{
			tangents[j] += T;
			bitangents[j] += B;
			weights[j] += w;
		}
	}

	for (int i = 0; i < n_vertices; i++)
	{
		mesh::vertex& v = mesh.vertices[i];
		glm::vec3& t = tangents[i];
		glm::vec3& b = bitangents[i];

//		v.normal /= weights[i];
		t /= weights[i];
		b /= weights[i];

		t -= glm::dot(v.normal, t) * v.normal;
		b -= glm::dot(v.normal, b) * v.normal + glm::dot(t, b) * t / glm::dot(t, t);

		v.normal = glm::normalize(v.normal);
		t = glm::normalize(t);
		b = glm::normalize(b);

		v.tangent = glm::vec4(t, glm::determinant(glm::mat3(t, b, v.normal)));
	}
}

}

#endif //DEEPLOM_MESH_HPP

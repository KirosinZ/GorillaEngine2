//
// Created by Kiril on 19.02.2023.
//

#include <fstream>
#include <sstream>
#include <optional>
#include "obj.h"

#include "glm/glm.hpp"


namespace gorilla::geom
{

bool operator==(const obj::index_triplet& l, const obj::index_triplet& r) noexcept
{
	return l.vi == r.vi
		&& l.vti == r.vti
		&& l.vni == r.vni;
}

obj::obj(
		vertex_info_t vertices,
		face_info_t faces,
		grouping_info_t groupings)
		: _vertices(std::move(vertices)), _faces(std::move(faces))
{
	if (_faces.face_offsets.empty() || _faces.face_indices.empty())
	{
		_faces.face_offsets.clear();
		_faces.face_indices.clear();
		_vertices.v.clear();
		_vertices.vt.clear();
		_vertices.vn.clear();
		return;
	}
	else if (_faces.face_offsets.size() == 1)
		throw std::runtime_error("Bullshit format");

	bool has_texcoords = _faces.face_indices[0].vti != -1;
	bool has_normals = _faces.face_indices[0].vni != -1;

	bool uniform_sides = true;
	int32_t n_sides = _faces.face_offsets[1] - _faces.face_offsets[0];

	for (int i = 0; i < n_faces(); i++)
	{
		const int32_t offset = _faces.face_offsets[i];
		const int32_t count = _faces.face_offsets[i + 1] - offset;

		if (uniform_sides && n_sides != count)
			uniform_sides = false;

		for (int j = offset; j < offset + count; j++)
		{
			if ((_faces.face_indices[j].vti != -1) != has_texcoords)
				throw std::runtime_error("Mismatch of texcoord indices: if one vertex has texcoord, all of them must have it.");

			if ((_faces.face_indices[j].vni != -1) != has_normals)
				throw std::runtime_error("Mismatch of normal indices: if one vertex has normal, all of them must have it.");
		}
	}

	_misc.uniform_face_side_count = uniform_sides;
	_misc.face_side_count = uniform_sides ? n_sides : 0;
	_misc.triangulated = uniform_sides ? n_sides == 3 : false;
}

obj obj::load_obj(const std::string& filename)
{
	using std::string_literals::operator""s;

	obj::vertex_info_t vertices;
	obj::face_info_t faces;

	std::optional<bool> has_texcoords = std::nullopt;
	std::optional<bool> has_normals = std::nullopt;

	int line_number = 0;
	const auto read_v = [&vertices, line_number] (std::stringstream& ss)
	{
		if (ss.eof())
			throw std::runtime_error("Not enough components in vertex position description (0/3) at line "s + std::to_string(line_number) + ".");
		float x = 0.0f;
		ss >> x;

		if (ss.eof())
			throw std::runtime_error("Not enough components in vertex position description (1/3) at line "s + std::to_string(line_number) + ".");
		float y = 0.0f;
		ss >> y;

		if (ss.eof())
			throw std::runtime_error("Not enough components in vertex position description (2/3) at line "s + std::to_string(line_number) + ".");
		float z = 0.0f;
		ss >> z;

		float w = 1.0f;
		if (!ss.eof())
		{
			ss >> w;
			if (!ss.eof())
				throw std::runtime_error("Too many components in vertex position description (max is 4) at line "s + std::to_string(line_number) + ".");
		}

		vertices.v.emplace_back(x, y, z, w);
	};

	const auto read_vt = [&vertices, line_number] (std::stringstream& ss)
	{
		if (ss.eof())
			throw std::runtime_error("Not enough components in texcoord description (0/2) at line "s + std::to_string(line_number) + ".");
		float u = 0.0f;
		ss >> u;

		if (ss.eof())
			throw std::runtime_error("Not enough components in texcoord description (1/2) at line "s + std::to_string(line_number) + ".");
		float v = 0.0f;
		ss >> v;

		float w = 0.0f;
		if (!ss.eof())
		{
			ss >> w;

			if (!ss.eof())
				throw std::runtime_error("Too many components in texcoord description (max is 3) at line "s + std::to_string(line_number) + ".");
		}

		vertices.vt.emplace_back(u, v, w);
	};

	const auto read_vn = [&vertices, line_number] (std::stringstream& ss)
	{
		if (ss.eof())
			throw std::runtime_error("Not enough components in normal description (0/3) at line "s + std::to_string(line_number) + ".");
		float x = 0.0f;
		ss >> x;

		if (ss.eof())
			throw std::runtime_error("Not enough components in normal description (1/3) at line "s + std::to_string(line_number) + ".");
		float y = 0.0f;
		ss >> y;

		if (ss.eof())
			throw std::runtime_error("Not enough components in normal description (2/3) at line "s + std::to_string(line_number) + ".");
		float z = 0.0f;
		ss >> z;

		if (!ss.eof())
			throw std::runtime_error("Too many components in normal description (max is 3) at line "s + std::to_string(line_number) + ".");

		vertices.vn.emplace_back(x, y, z);
	};

	const auto read_f = [&vertices, &faces, &has_texcoords, &has_normals, line_number] (std::stringstream& ss)
	{
		obj::index_triplet triplet;

		std::string snippet;
		uint32_t cnt = 0;
		while (!ss.eof())
		{
			++cnt;
			ss >> snippet;
			std::stringstream sss(snippet);
			char delim;
			sss >> triplet.vi;
			if (triplet.vi < 0)
				triplet.vi += vertices.v.size();
			else
				triplet.vi -= 1;
			if (!sss.eof())
			{
				sss >> delim;
				if (sss.eof())
					throw std::runtime_error("Missing texcoord/normal index after \"/\" at line "s + std::to_string(line_number) + ".");

				sss >> delim;
				if (delim != '/')
				{
					sss.unget();
					sss >> triplet.vti;
					if (triplet.vti < 0)
						triplet.vti += vertices.vt.size();
					else
						triplet.vti -= 1;
					if (sss.eof())
						goto assignment;

					sss >> delim;
				}
				if (sss.eof())
					throw std::runtime_error("Missing normal index after \"/\" at line "s + std::to_string(line_number) + ".");

				sss >> triplet.vni;
				if (triplet.vni < 0)
					triplet.vni += vertices.vn.size();
				else
					triplet.vni -= 1;

				if (!sss.eof())
					throw std::runtime_error("Too many indices specified (max is 3) at line"s + std::to_string(line_number) + ".");
			}
assignment:
			if (!has_texcoords.has_value())
			{
				has_texcoords = triplet.vti != -1;
			}
			else if (has_texcoords.value() != (triplet.vti != -1))
				throw std::runtime_error("Mismatch of texcoord indices: if one vertex has texcoord, all of them must have it. At line "s + std::to_string(line_number) + ".");

			if (!has_normals.has_value())
			{
				has_normals = triplet.vni != -1;
			}
			else if (has_normals.value() != (triplet.vni != -1))
				throw std::runtime_error("Mismatch of normal indices: if one vertex has normal, all of them must have it. At line "s + std::to_string(line_number) + ".");

			faces.face_indices.push_back(triplet);

			ss >> std::ws;
		}
		if (cnt < 3)
			throw std::runtime_error("Not enough index descriptions in a face (min is 3) at line "s + std::to_string(line_number) + ".");

		if (faces.face_offsets.empty())
			faces.face_offsets.push_back(0);
		faces.face_offsets.push_back(faces.face_offsets.back() + cnt);
	};


	std::ifstream fs(filename, std::ios::in);
	if (!fs.is_open())
		throw std::runtime_error(std::string("Couldn't open file: \"") + filename + "\".");

	while(!fs.eof())
	{
		++line_number;
		std::string line;
		std::getline(
				fs,
				line);

		if (line.empty())
			continue;

		std::stringstream ss(line);

		std::string descriptor;
		ss >> descriptor;

		if (descriptor == "#"
		|| descriptor == "vp"
		|| descriptor == "l"
		|| descriptor == "o"
		|| descriptor == "g"
		|| descriptor == "mtllib"
		|| descriptor == "usemtl"
		|| descriptor == "s")
			continue;

		if (descriptor == "v")
			read_v(ss);
		else if (descriptor == "vt")
			read_vt(ss);
		else if (descriptor == "vn")
			read_vn(ss);
		else if (descriptor == "f")
			read_f(ss);
		else
			throw std::runtime_error(std::string("Unknown token \"") + descriptor + "\" at line " + std::to_string(line_number) + ".");
	}

	if (faces.face_offsets.empty() || faces.face_indices.empty())
	{
		faces.face_indices.clear();
		faces.face_offsets.clear();

		vertices.v.clear();
		vertices.vt.clear();
		vertices.vn.clear();
	}
	else
	{
		if (!has_texcoords.value())
			vertices.vt.clear();

		if (!has_normals.value())
			vertices.vn.clear();
	}

	return obj(std::move(vertices), std::move(faces));
}

bool operator==(const obj& l, const obj& r) noexcept
{
	// TODO: Implementation needed
	return l._vertices.v == r._vertices.v
		&& l._vertices.vt == r._vertices.vt
		&& l._vertices.vn == r._vertices.vn
		&& l._faces.face_indices == r._faces.face_indices
		&& l._faces.face_offsets == r._faces.face_offsets;
}

obj triangulate_obj(const obj& obj)
{
	obj::vertex_info_t vertex_info = obj.vertex_info();
	obj::face_info_t face_info = obj.face_info();

	face_info.face_indices = triangulate(face_info.face_indices, face_info.face_offsets);
	const int n_triangles = face_info.face_indices.size() / 3;
	face_info.face_offsets.clear();
	for (int i = 0; i < n_triangles + 1; i++)
	{
		face_info.face_offsets.push_back(3 * i);
	}

	return {vertex_info, face_info};
}

std::vector<glm::vec3> calculate_normals(const std::vector<glm::vec4>& positions, const std::vector<obj::index_triplet>& triangle_indices)
{
	std::vector<glm::vec3> res;
	res.resize(positions.size());

	std::vector<float> weights;
	weights.resize(positions.size());

	const int n_triangles = triangle_indices.size() / 3;
	for (int i = 0; i < n_triangles; i++)
	{
		const glm::vec4& p0 = positions[3 * i + 0];
		const glm::vec4& p1 = positions[3 * i + 1];
		const glm::vec4& p2 = positions[3 * i + 2];
		glm::vec3 v1 = p1 / p1.w - p0 / p0.w;
		glm::vec3 v2 = p2 / p2.w - p0 / p0.w;

		glm::vec3 n = glm::cross(v1, v2);

		const float area = n.length() / 2;
		n /= 2.0f;

		res[3 * i + 0] += n;
		res[3 * i + 1] += n;
		res[3 * i + 2] += n;

		weights[3 * i + 0] += area;
		weights[3 * i + 1] += area;
		weights[3 * i + 2] += area;
	}

	for (int i = 0; i < positions.size(); i++)
	{
		res[i] = glm::normalize(res[i] / weights[i]);
	}

	return res;
}

} // gorilla::obj
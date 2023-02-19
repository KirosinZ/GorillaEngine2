//
// Created by Kiril on 19.02.2023.
//

#include <fstream>
#include <sstream>
#include "geom.h"

namespace gorilla::geom
{

obj obj::load_obj(const std::string& filename)
{
	int current_object_index = 0;
	std::vector<std::string> shapenames = { "" };
	std::vector<shape> shapes = { {} };

	const auto read_v = [&shapes, current_object_index] (std::stringstream& ss)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		ss >> x >> y >> z;
		if (!ss.eof())
		{
			float w;
			ss >> w;
			float wr = 1.0f / w;
			x *= wr;
			y *= wr;
			z *= wr;
		}

		shapes[current_object_index].v.emplace_back(x, y, z);
	};

	const auto read_vt = [&shapes, current_object_index] (std::stringstream& ss)
	{
		float u = 0.0f;
		float v = 0.0f;
		float w = 0.0f;
		ss >> u >> v;
		if (!ss.eof())
		{
			ss >> w;
		}

		shapes[current_object_index].vt.emplace_back(u, v, w);
	};

	const auto read_vn = [&shapes, current_object_index] (std::stringstream& ss)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		ss >> x >> y >> z;

		shapes[current_object_index].vn.emplace_back(x, y, z);
	};

	const auto read_f = [&shapes, current_object_index] (std::stringstream& ss)
	{
		std::string snippet;
		int cnt = shapes[current_object_index].polygon_offsets.back();
		while (!ss.eof())
		{
			++cnt;
			ss >> snippet;
			std::stringstream sss(snippet);
			int i;
			char delim;
			sss >> i;
			shapes[current_object_index].polygon_indices_v.push_back(i);
			if (sss.eof())
				continue;

			sss >> delim;
			sss >> delim;
			if (delim != '/')
			{
				sss.unget();
				sss >> i;
				shapes[current_object_index].polygon_indices_vt.push_back(i);
				if (sss.eof())
					continue;

				sss >> delim;
			}
			sss >> i;
			shapes[current_object_index].polygon_indices_vn.push_back(i);
		}
		shapes[current_object_index].polygon_offsets.push_back(cnt);
	};

	obj res;

	std::ifstream fs(filename, std::ios::in);
	if (!fs.is_open())
		throw std::runtime_error("Couldn't open file");

	while(!fs.eof())
	{
		std::string line;
		std::getline(
				fs,
				line);

		std::stringstream ss(line);

		std::string descriptor;
		ss >> descriptor;

		if (descriptor == "v")
			read_v(ss);
		if (descriptor == "vt")
			read_vt(ss);
		if (descriptor == "vn")
			read_vn(ss);
		if (descriptor == "f")
			read_f(ss);
		else
			continue;
	}

	res._shapenames = shapenames;
	res._shapes = shapes;

	return res;
}

std::vector<std::string> obj::shape_names() const
{
	return _shapenames;
}

std::vector<obj::shape> obj::shapes() const
{
	return _shapes;
}

const obj::shape& obj::shape_by_name(const std::string& name) const
{
	const size_t index = std::distance(
			_shapenames.begin(),
			std::find(
					_shapenames.begin(),
					_shapenames.end(),
					name));

	return _shapes[index];
}

} // gorilla::geom
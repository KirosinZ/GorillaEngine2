//
// Created by Kiril on 03.05.2023.
//

#include "obj_detail.h"

namespace gorilla::asset::detail
{

glm::vec4 parse_vertex(std::stringstream& stream)
{
	glm::vec4 res{};
	res.w = 1.0f;

	stream >> std::ws;

	if (stream.eof())
		throw std::runtime_error("Not enough components in vertex position description (0/3)");
	stream >> res.x;

	if (stream.eof())
		throw std::runtime_error("Not enough components in vertex position description (1/3)");
	stream >> res.y;

	if (stream.eof())
		throw std::runtime_error("Not enough components in vertex position description (2/3)");
	stream >> res.z;

	if (stream.eof())
		return res;

	stream >> res.w;

	stream >> std::ws;

	if (!stream.eof())
		throw std::runtime_error("Too many components in vertex position description (max is 4)");

	return res;
}

glm::vec3 parse_tex_coord(std::stringstream& stream)
{
	glm::vec3 res{};

	stream >> std::ws;

	if (stream.eof())
		throw std::runtime_error("Not enough components in texture coordinate description (0/3)");
	stream >> res.x;

	if (stream.eof())
		throw std::runtime_error("Not enough components in texture coordinate description (1/3)");
	stream >> res.y;

	if (stream.eof())
		return res;

	stream >> res.z;

	stream >> std::ws;

	if (!stream.eof())
		throw std::runtime_error("Too many components in texture coordinate description (max is 3)");

	return res;
}

glm::vec3 parse_normal(std::stringstream& stream)
{
	glm::vec3 res{};

	stream >> std::ws;

	if (stream.eof())
		throw std::runtime_error("Not enough components in normal description (0/3)");
	stream >> res.x;

	if (stream.eof())
		throw std::runtime_error("Not enough components in normal description (1/3)");
	stream >> res.y;

	if (stream.eof())
		throw std::runtime_error("Not enough components in normal description (2/3)");
	stream >> res.z;

	stream >> std::ws;

	if (!stream.eof())
		throw std::runtime_error("Too many components in normal description (max is 3)");

	return res;
}

std::array<int, 3> parse_face_vertex(std::stringstream& stream)
{
	std::array<int, 3> res = { -1, -1, -1 };

	stream >> std::ws;

	if (stream.eof())
		throw std::runtime_error("Missing vertex position index");

	stream >> res[0];
	if (res[0] < 0)
		throw std::runtime_error("Negative indices not supported");
	--res[0];

	stream >> std::ws;
	if (stream.eof())
		return res;

	char delim{};
	stream >> delim;
	if (delim != '/')
		throw std::runtime_error("Bad delimeter in face description");

	stream >> std::ws;
	if (stream.eof())
		throw std::runtime_error("Missing texture coordinate index after \"/\"");

	stream >> delim;
	if (delim == '/')
	{
		stream >> std::ws;
		if (stream.eof())
			throw std::runtime_error("Missing normal index after \"//\"");

		stream >> res[2];
		if (res[2] < 0)
			throw std::runtime_error("Negative indices not supported");
		--res[2];

		stream >> std::ws;
		if (!stream.eof())
			throw std::runtime_error("Extra characters at the end of stream");

		return res;
	}

	stream.unget();

	stream >> res[1];
	if (res[1] < 0)
		throw std::runtime_error("Negative indices not supported");
	--res[1];

	stream >> std::ws;
	if (stream.eof())
		return res;

	stream >> delim;
	if (delim != '/')
		throw std::runtime_error("Bad delimeter in face description");

	stream >> std::ws;
	if (stream.eof())
		throw std::runtime_error("Missing normal index after \"/\"");

	stream >> res[2];
	if (res[2] < 0)
		throw std::runtime_error("Negative indices not supported");
	--res[2];

	stream >> std::ws;
	if (!stream.eof())
		throw std::runtime_error("Extra characters at the end of stream");

	return res;
}
std::vector<std::array<int, 3>> parse_face(std::stringstream& stream)
{
	std::vector<std::array<int, 3>> res;

	int vt_cnt = 0;
	int vn_cnt = 0;

	stream >> std::ws;
	while(!stream.eof())
	{
		std::string snippet;
		stream >> snippet;
		std::stringstream snippetstream(snippet);

		const std::array<int, 3>& triplet = parse_face_vertex(snippetstream);

		if (triplet[1] != -1)
			++vt_cnt;

		if (triplet[2] != -1)
			++vn_cnt;

		res.push_back(triplet);

		stream >> std::ws;
	}

	if (res.size() < 3)
		throw std::runtime_error("Not enough face indices for a face description (Min is 3)");

	if (vt_cnt != 0 && vt_cnt != res.size())
		throw std::runtime_error("Texture coordinates are not specified for all vertices");

	if (vn_cnt != 0 && vn_cnt != res.size())
		throw std::runtime_error("Normals are not specified for all vertices");

	return res;
}

}
//
// Created by Kiril on 05.04.2023.
//

#ifndef DEEPLOM_GEOMETRY_H
#define DEEPLOM_GEOMETRY_H

#include <vector>

#include <glm/glm.hpp>


namespace gorilla
{

struct geometry
{
public:
	std::vector<glm::vec3> _positions;
	std::vector<glm::vec3> _texcoords;
	std::vector<glm::vec3> _normals;
	std::vector<glm::vec3> _tangents;

	std::vector<uint32_t> _position_indices;
	std::vector<uint32_t> _texcoord_indices;
};



} // gorilla

#endif //DEEPLOM_GEOMETRY_H

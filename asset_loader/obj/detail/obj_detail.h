//
// Created by Kiril on 03.05.2023.
//

#ifndef DEEPLOM_OBJ_DETAIL_H
#define DEEPLOM_OBJ_DETAIL_H

#include <glm/glm.hpp>
#include <sstream>
#include <vector>
#include <array>


namespace gorilla::asset::detail
{

glm::vec4 parse_vertex(std::stringstream& stream);
glm::vec3 parse_tex_coord(std::stringstream& stream);
glm::vec3 parse_normal(std::stringstream& stream);

std::array<int, 3> parse_face_vertex(std::stringstream& stream);
std::vector<std::array<int, 3>> parse_face(std::stringstream& stream);

}

#endif //DEEPLOM_OBJ_DETAIL_H

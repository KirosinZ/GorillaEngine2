//
// Created by Kiril on 19.02.2023.
//

#ifndef DEEPLOM_GEOM_H
#define DEEPLOM_GEOM_H

#include <vector>
#include <string>
#include <map>

#include <glm/vec3.hpp>

namespace gorilla::geom
{

class obj
{
public:
	struct shape
	{
		std::vector<glm::vec3> v;
		std::vector<glm::vec3> vt;
		std::vector<glm::vec3> vn;

		std::vector<uint32_t> polygon_indices_v;
		std::vector<uint32_t> polygon_indices_vt;
		std::vector<uint32_t> polygon_indices_vn;
		std::vector<uint32_t> polygon_offsets = { 0 };
	};

	obj(const obj& copy) = default;
	obj& operator=(const obj& copy) = default;

	obj(obj&& move) = default;
	obj& operator=(obj&& move) = default;

	static obj load_obj(const std::string& filename);

	std::vector<std::string> shape_names() const;
	std::vector<shape> shapes() const;

	const shape& shape_by_name(const std::string& name) const;
private:
	obj() = default;

	std::vector<std::string> _shapenames;
	std::vector<shape> _shapes;
};

} // gorilla::geom

#endif //DEEPLOM_GEOM_H

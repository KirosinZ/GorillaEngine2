//
// Created by Kiril on 12.04.2023.
//

#ifndef DEEPLOM_SCENE_H
#define DEEPLOM_SCENE_H

#include <engine/mesh/mesh.hpp>
#include <engine/application/texture.h>
#include <engine/application/geombuffer.h>

#include <engine/mesh/light.hpp>

namespace gorilla::engine
{

struct scene
{
	struct prop_description
	{
		std::string obj_filename;
		std::string texture_filename;
		std::string normal_filename;
		std::string roughness_filename;

		glm::mat4 model = glm::mat4(1.0f);
	};

	struct prop
	{
		mesh mesh;
		engine::geom_buffer geom_buffer;
		engine::texture texture;
		engine::texture normal;
		engine::texture roughness;

		glm::mat4 model = glm::mat4(1.0f);
	};

	lights lights;
	std::vector<prop_description> props;
};

} // gorilla::engine

#endif //DEEPLOM_SCENE_H

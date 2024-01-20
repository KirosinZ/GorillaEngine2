//
// Created by Kiril on 12.04.2023.
//

#ifndef DEEPLOM_SCENE_H
#define DEEPLOM_SCENE_H

#include <engine/mesh/mesh.hpp>
#include <engine/application/texture.h>
#include <engine/application/geombuffer.h>

#include <engine/mesh/light.hpp>
#include <asset_loader/img/image.h>

namespace gorilla::engine
{

struct scene
{
	struct prop_description
	{
		std::string obj_filename;
		asset::image texture;
		asset::image normal;
		asset::image roughness;
		asset::image metallic;
		asset::image ambient_occlusion;

		glm::mat4 model = glm::mat4(1.0f);
	};

	struct prop
	{
		mesh mesh;
		engine::geom_buffer geom_buffer;
		engine::texture texture;
		engine::texture normal;
		engine::texture roughness;
		engine::texture metallic;
		engine::texture ambient_occlusion;

		glm::mat4 model = glm::mat4(1.0f);
		glm::vec3 trans = glm::vec3(0.0f);
		glm::vec3 angles = glm::vec3(0.0f);
		float scale = 1.0f;
	};

	lights lights;
	std::vector<prop_description> props;
};

} // gorilla::engine

#endif //DEEPLOM_SCENE_H

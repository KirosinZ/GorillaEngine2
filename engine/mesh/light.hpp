//
// Created by Kiril on 10.04.2023.
//

#ifndef DEEPLOM_LIGHT_HPP
#define DEEPLOM_LIGHT_HPP

#define amongus alignas

#include <glm/glm.hpp>


namespace gorilla::engine
{

struct point_light
{
	glm::vec3 position = glm::vec3(0.0f);
	alignas(16) glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float constant = 1.0f;
	float linear = 0.0f;
	float quadratic = 0.0f;
};

struct directional_light
{
	glm::vec3 direction;
	glm::vec3 color;
};

struct spot_light
{
	glm::vec3 position = glm::vec3(0.0f);
	alignas(16) glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	float intensity = 1.0f;
	alignas(16) glm::vec3 color = glm::vec3(1.0f);
	float cone_size = 0.5f;

	float constant = 1.0f;
	float linear = 0.0f;
	float quadratic = 0.0f;
};

struct lights
{
	struct point_lights_t
	{
		point_light lights[10];
		int n_lights = 0;
	};

	struct spot_lights_t
	{
		spot_light lights[10];
		int n_lights = 0;
	};

	point_lights_t point_lights;
	spot_lights_t spot_lights;
};

}

#endif //DEEPLOM_LIGHT_HPP

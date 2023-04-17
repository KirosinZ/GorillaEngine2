//
// Created by Kiril on 30.01.2023.
//

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace gorilla
{

camera::camera(
	const glm::vec3& pos,
	const glm::vec3& fwd,
	const glm::vec3& global_up)
	: position(pos),
	  fwd(fwd),
	  global_up(global_up)
{
	right = glm::cross(fwd, global_up);
	up = glm::cross(right, fwd);
}

void camera::rotate(float yaw, float pitch)
{
	this->pitch = glm::radians(pitch);
	this->yaw = -glm::radians(yaw);

	glm::mat4 id = glm::mat4(1.0f);
	id = glm::rotate(id, this->pitch, right);
	id = glm::rotate(id, this->yaw, global_up);
	fwd = id * glm::vec4(fwd, 1.0f);

	right = glm::cross(fwd, global_up);
	up = glm::cross(right, fwd);
}

glm::mat4 camera::view() const
{
	return glm::lookAt(position, position + fwd, up);
}

} // gorilla
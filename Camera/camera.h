//
// Created by Kiril on 30.01.2023.
//

#ifndef DEEPLOM_CAMERA_H
#define DEEPLOM_CAMERA_H

#include <glm/glm.hpp>

namespace gorilla
{

class camera
{
public:
	glm::vec3 position;

	camera(
		const glm::vec3& pos,
		const glm::vec3& fwd,
		const glm::vec3& global_up = glm::vec3(0.0f, 1.0f, 0.0f));

	void rotate(float yaw, float pitch);

	glm::mat4 view() const;

private:
	float yaw = 0.0f;
	float pitch = 0.0f;

	glm::vec3 fwd;
	glm::vec3 up;
	glm::vec3 right;

	glm::vec3 global_up;
};

} // gorilla

#endif //DEEPLOM_CAMERA_H

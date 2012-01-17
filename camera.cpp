#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdio>

const glm::mat4 camera_t::translation_matrix() const{
	return glm::translate(glm::mat4(1.0f), position_);
}

const glm::mat4 camera_t::rotation_matrix() const{
	return glm::mat4_cast(orientation_);
}

const glm::mat4 camera_t::matrix() const{
	return translation_matrix()*rotation_matrix();
}


void camera_t::relative_move(const glm::vec3 &move) {
	position_+= glm::vec3(rotation_matrix()*glm::vec4(move,1.f));
}

void camera_t::relative_rotate(const glm::vec3 &axis, const float &angle) {
	printf("Rotate: %f\n", angle);
	orientation_ = glm::rotate(orientation_, angle, axis);
}

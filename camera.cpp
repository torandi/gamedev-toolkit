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

const glm::vec3 camera_t::look_at() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.0, 0.0, 1.0, 1.0))+position_;
}
const glm::vec3 camera_t::up() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.0, 1.0, 0.0, 1.0));
}

void camera_t::relative_move(const glm::vec3 &move) {
	position_+= orient_vector(move);
}

void camera_t::absolute_rotate(const glm::vec3 &axis, const float &angle) {
	orientation_ = glm::rotate(orientation_, angle, orient_vector(axis*1.f));
}

void camera_t::relative_rotate(const glm::vec3 &axis, const float &angle) {
	orientation_ = glm::rotate(orientation_, angle, axis);
}

glm::vec3 camera_t::orient_vector(const glm::vec3 &vec) const {
	return glm::vec3(rotation_matrix()*glm::vec4(vec, 1.f));
}

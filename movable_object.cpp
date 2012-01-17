#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdio>

MovableObject::~MovableObject() { }	

const glm::mat4 MovableObject::translation_matrix() const{
	return glm::translate(glm::mat4(1.0f), position_);
}

const glm::mat4 MovableObject::rotation_matrix() const{
	return glm::mat4_cast(orientation_);
}

const glm::mat4 MovableObject::matrix() const{
	return translation_matrix()*rotation_matrix();
}

void MovableObject::relative_move(const glm::vec3 &move) {
	position_+= orient_vector(move);
}

void MovableObject::absolute_rotate(const glm::vec3 &axis, const float &angle) {
	orientation_ = glm::rotate(orientation_, angle, orient_vector(axis*1.f));
}

void MovableObject::absolute_move(const glm::vec3 &move) {
	position_ += move;
}

void MovableObject::relative_rotate(const glm::vec3 &axis, const float &angle) {
	orientation_ = glm::rotate(orientation_, angle, axis);
}

glm::vec3 MovableObject::orient_vector(const glm::vec3 &vec) const {
	return glm::vec3(rotation_matrix()*glm::vec4(vec, 1.f));
}

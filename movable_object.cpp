#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdio>

MovableObject::MovableObject() : orientation_(1.f, 0.f, 0.f,0.f) {
	rotation_matrix_dirty_ = true;
	translation_matrix_dirty_ = true;
	matrix_dirty_ = true;
};

MovableObject::MovableObject(glm::vec3 position) : position_(position), orientation_(1.f, 0.f, 0.f,0.f) { 
	MovableObject();	
}

MovableObject::~MovableObject() { }	

const glm::mat4 MovableObject::translation_matrix() const{
	if(translation_matrix_dirty_) {
		trans_mat_ = glm::translate(glm::mat4(1.0f), position());
		translation_matrix_dirty_ = false;
	}
	return trans_mat_;
}

const glm::mat4 MovableObject::rotation_matrix() const{
	if(rotation_matrix_dirty_) {
		rot_matrix_ = glm::mat4_cast(orientation_);
		rotation_matrix_dirty_ = false;
	}
	return rot_matrix_;
}

const glm::mat4 MovableObject::matrix() const{
	if(matrix_dirty_) {
		matrix_dirty_ = false;
		matrix_ = translation_matrix()*rotation_matrix();
	}
	return matrix_;
}

void MovableObject::relative_move(const glm::vec3 &move) {
	translation_matrix_dirty_ = true;
	position_+= orient_vector(move);
}

void MovableObject::absolute_rotate(const glm::vec3 &axis, const float &angle) {
	rotation_matrix_dirty_ = true;
	orientation_ = glm::rotate(orientation_, angle, orient_vector(axis*1.f));
}

void MovableObject::absolute_move(const glm::vec3 &move) {
	translation_matrix_dirty_ = true;
	position_ += move;
}

void MovableObject::relative_rotate(const glm::vec3 &axis, const float &angle) {
	rotation_matrix_dirty_ = true;
	orientation_ = glm::rotate(orientation_, angle, axis);
}

glm::vec3 MovableObject::orient_vector(const glm::vec3 &vec) const {
	return glm::vec3(rotation_matrix()*glm::vec4(vec, 1.f));
}

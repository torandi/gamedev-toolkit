#include "move_group.h"
#include <vector>

MoveGroup::~MoveGroup() {
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		delete (*it);
	}
}

void MoveGroup::relative_move(const glm::vec3 &move) {
	MovableObject::relative_move(move);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->relative_move(move);
	}
}

void MoveGroup::relative_rotate(const glm::vec3 &axis, const float &angle) {
	MovableObject::relative_rotate(axis, angle);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->relative_rotate(axis, angle);
	}
}

void MoveGroup::absolute_rotate(const glm::vec3 &axis, const float &angle) {
	MovableObject::absolute_rotate(axis, angle);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->absolute_rotate(axis, angle);
	}
}

void MoveGroup::absolute_move(const glm::vec3 &move) {
	MovableObject::absolute_move(move);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->absolute_move(move);
	}
}

void MoveGroup::set_position(const glm::vec3 &pos) {
	MovableObject::set_position(pos);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->set_position(pos);
	}
}
void MoveGroup::set_rotation(const glm::vec3 &axis, const float angle) {
	MovableObject::set_rotation(axis, angle);
	for(std::vector<MovableObject*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->set_rotation(axis, angle);
	}
}

MovableObject* MoveGroup::operator[](int index) const {
	return objects_[index];
}

void MoveGroup::add_object(MovableObject* obj) {
	objects_.push_back(obj);
}

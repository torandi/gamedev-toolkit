#ifndef MOVE_GROUP_H
#define MOVE_GROUP_H

#include "movable_object.h"
#include <vector>

class MoveGroup : public MovableObject {
	std::vector<MovableObject*> objects_;

public:
	MoveGroup() : MovableObject() { } 

	virtual ~MoveGroup();

	virtual void relative_move(const glm::vec3 &move);
	virtual void relative_rotate(const glm::vec3 &axis, const float &angle);

	virtual void absolute_rotate(const glm::vec3 &axis, const float &angle);
	virtual void absolute_move(const glm::vec3 &move);

	virtual void set_position(const glm::vec3 &pos);
	virtual void set_rotation(const glm::vec3 &axis, const float angle);

	MovableObject* operator[](int index) const;
	void add_object(MovableObject* obj);

};
#endif

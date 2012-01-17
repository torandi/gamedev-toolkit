#ifndef RENDER_GROUP_H
#define RENDER_GROUP_H

#include "movable_object.h"
#include <vector>

class Renderer; //Forward declaration

class RenderGroup : public MovableObject {
	std::vector<RenderGroup*> objects_;

public:
	glm::vec3 scale;

	RenderGroup() : MovableObject(), scale(1.f) {};
	RenderGroup(glm::vec3 position) : MovableObject(position) , scale(1.f){};
	virtual ~RenderGroup();

	RenderGroup* operator[](int index) const;

	void add_object(RenderGroup* obj);

	virtual void render(double dt, Renderer * renderer);
	virtual const glm::mat4 matrix() const;

};

#endif

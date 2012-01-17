#include "render_group.h"
#include "renderer.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderGroup::~RenderGroup() {
	for(std::vector<RenderGroup*>::iterator it=objects_.begin(); it!= objects_.end(); ++it) {
		delete *it;
	}
}

RenderGroup* RenderGroup::operator[](int index) const {
	return objects_[index];
}

void RenderGroup::add_object(RenderGroup* obj) {
	objects_.push_back(obj);
}

const glm::mat4 RenderGroup::matrix() const {
	//Also apply scale:
	return MovableObject::matrix() * glm::scale(glm::mat4(1.0f), scale);
}

void RenderGroup::render(double dt, Renderer * renderer) {
	renderer->modelViewMatrix.Push();
	
	renderer->modelViewMatrix.ApplyMatrix(matrix());

	for(std::vector<RenderGroup*>::iterator it=objects_.begin(); it!=objects_.end(); ++it) {
		(*it)->render(dt, renderer);
	}

	renderer->modelViewMatrix.Pop();
}


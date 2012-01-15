#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <string>
#include <assimp/aiScene.h>

struct RenderObject {
	const aiScene* scene;

	RenderObject(std::string model);

	void render(double dt);
};

#endif

#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <string>
#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <map>
#include <glload/gl_3_3.h>
#include <glm/glm.hpp>

struct RenderObject {
	const aiScene* scene;
	glm::vec3 scene_min, scene_max, scene_center;
	std::map<const aiNode*, GLuint*> node_vao;
	std::map<const aiMesh*, GLuint*> mesh_buffers;

	RenderObject(std::string model);

	void pre_render();
	void recursive_pre_render(const aiNode* node);
	void recursive_render(const aiNode* node, double dt);
	void render(double dt);
private:
	void get_bounding_box_for_node (const struct aiNode* nd,	struct aiVector3D* min, struct aiVector3D* max, struct aiMatrix4x4* trafo);
	void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
};

#endif

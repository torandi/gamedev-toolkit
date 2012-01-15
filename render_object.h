#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <string>
#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <map>
#include <vector>
#include <glload/gl_3_3.h>
#include <glm/glm.hpp>

struct RenderObject {
	const aiScene* scene;
	glm::vec3 scene_min, scene_max, scene_center;

	struct mesh_data_t {
		GLuint vb, ib; //vertex buffer, index buffer
		GLenum draw_mode;
		unsigned int num_indices;
		unsigned int mtl_index;
	};

	struct vertex_t {
		float pos_x;
		float pos_y;
		float pos_z;
		float texCoord_x;
		float texCoord_y;
		float normal_x;
		float normal_y;
		float normal_z;

		vertex_t(const aiVector3D* pos,const  aiVector3D* texCoord,const  aiVector3D* normal) {
			pos_x = pos->x;
			pos_y = pos->y;
			pos_z = pos->z;
			texCoord_x = texCoord->x;
			texCoord_y = texCoord->y;
			normal_x = normal->x;
			normal_y = normal->y;
			normal_z = normal->z;
		}
	};

	struct material_t {
		GLuint texture;
	};

	std::vector<material_t> materials;

	std::map<const aiMesh*, mesh_data_t > mesh_data;

	RenderObject(std::string model);

	void pre_render();
	void recursive_pre_render(const aiNode* node);
	void recursive_render(const aiNode* node, double dt);
	void render(double dt);
private:
	float rotation;

	void get_bounding_box_for_node (const struct aiNode* nd,	struct aiVector3D* min, struct aiVector3D* max, struct aiMatrix4x4* trafo);
	void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
};

#endif

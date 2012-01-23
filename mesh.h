#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <vector>
#include <glload/gl_3_3.h>


class Mesh {
public:
	

	struct vertex_t {
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	Mesh(const std::vector<vertex_t> &vertices, const std::vector<unsigned int> &indices);
	~Mesh();

	void generate_normals();
	void generate_tangents_and_bitangents();
	void ortonormalize_tangent_space();
	//The mesh becommes immutable when vbos have been generated
	void generate_vbos();
	void render();
	unsigned long num_faces() { return num_faces_; };
private:
	GLenum buffers_[2]; //0:vertex buffer, 1: index buffer
	bool vbos_generated_;
	unsigned long num_faces_;
	std::vector<vertex_t> vertices_;
	std::vector<unsigned int> indices_;

	void verify_immutable(const char * where); //Checks that vbos_generated == false


};

#endif

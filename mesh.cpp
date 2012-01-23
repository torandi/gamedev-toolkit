#include "mesh.h"
#include "renderer.h"

#include <cstdio>
#include <glm/glm.hpp>
#include <vector>
#include <cassert>
#include <glload/gl_3_3.h>

Mesh::Mesh(const std::vector<vertex_t> &vertices, const std::vector<unsigned int> &indices) :
	vbos_generated_(false),vertices_(vertices), indices_(indices)	{
	assert((indices.size()%3)==0);
}

Mesh::~Mesh() {
	if(vbos_generated_)
		glDeleteBuffers(2, buffers_);
}

void Mesh::generate_normals() {
	verify_immutable("calculate_normals()");

	for(unsigned int i=0; i<indices_.size(); i+=3) {
		unsigned int tri[3] = {indices_[i], indices_[i+1], indices_[i+2]};
		vertex_t * face[3] = {&vertices_[tri[0]], &vertices_[tri[1]], &vertices_[tri[2]]};
		glm::vec3 v1 = face[1]->position-face[0]->position;
		glm::vec3 v2 = face[2]->position-face[0]->position;
		glm::vec3 normal = glm::cross(v1, v2);
		for(int f=0; f<3;++f) {
			face[f]->normal += normal;
		}
	}
}

//This method orgonormalizes the tangent space
void Mesh::ortonormalize_tangent_space() {
	for(std::vector<vertex_t>::iterator it=vertices_.begin(); it!=vertices_.end(); ++it) {
		it->normal = glm::normalize(it->normal);	
		//Make sure tangent is ortogonal to normal (and normalized)
		it->tangent = glm::normalize(it->tangent - it->normal*glm::dot(it->normal, it->tangent));
		//Normalize bitangent
		it->bitangent = glm::normalize(it->bitangent);
		//Make sure tangent space is right handed:
		glm::vec3 new_bitangent = glm::cross(it->normal, it->tangent);
		if(glm::dot(glm::cross(it->normal, it->tangent), new_bitangent) < 0.0f) {
			it->tangent *= -1.0f;
		}
		it->bitangent = new_bitangent;

	}
}

void Mesh::generate_tangents_and_bitangents() {
	for(unsigned int i=0; i<indices_.size(); i+=3) {
		unsigned int tri[3] = {indices_[i], indices_[i+1], indices_[i+2]};
		vertex_t * face[3] = {&vertices_[tri[0]], &vertices_[tri[1]], &vertices_[tri[2]]};
		glm::vec3 v1 = face[1]->position-face[0]->position;
		glm::vec3 v2 = face[2]->position-face[0]->position;
		glm::vec2 uv1 = face[1]->texCoord-face[0]->texCoord;
		glm::vec2 uv2 = face[2]->texCoord-face[0]->texCoord;

		float r=1.f / (uv1.x * uv2.y - uv1.y * uv2.x);
		glm::vec3 tangent = (v1 * uv2.y - v2 * uv1.y)*r;
		glm::vec3 bitangent = (v2 * uv1.x - v1 * uv2.x)*r;

		for(int f=0; f<3; ++f) {
			face[f]->tangent += tangent;
			face[f]->bitangent += bitangent;
		}
	}
}

void Mesh::verify_immutable(const char * where) {
	if(vbos_generated_) {
		fprintf(stderr,"Mesh::%s can not be used after vertex buffers have been generated\n", where);
		assert(false);
	}
}

void Mesh::generate_vbos() {
	verify_immutable("generate_vbos()");

	//Upload data:
	glGenBuffers(2, buffers_);
	Renderer::checkForGLErrors("Mesh::generate_vbos(): gen buffers");

	glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*vertices_.size(), &vertices_.front(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	Renderer::checkForGLErrors("Mesh::generate_vbos(): fill array buffer");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices_.size(), &indices_.front(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	Renderer::checkForGLErrors("Mesh::generate_vbos(): fill element array buffer");

	num_faces_ = indices_.size();

	vbos_generated_ = true;
}

void Mesh::render() {
	glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)+sizeof(glm::vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (2*sizeof(glm::vec3)+sizeof(glm::vec2)));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (3*sizeof(glm::vec3)+sizeof(glm::vec2)));

	glDrawElements(GL_TRIANGLES, num_faces_, GL_UNSIGNED_INT, 0);	

	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

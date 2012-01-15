#include "render_object.h"
#include "render.h"
#include <string>
#include <cstdio>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <glm/glm.hpp>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

RenderObject::RenderObject(std::string model) {
	scene = aiImportFile( model.c_str(),  aiProcessPreset_TargetRealtime_MaxQuality);
	if(scene != 0) {
		printf("Loaded model %s: \nMeshes: %d\nTextures: %d\nMaterials: %d\n",model.c_str(), scene->mNumMeshes, scene->mNumTextures, scene->mNumMaterials);

		//Get bounds:
		aiVector3D s_min, s_max;
		get_bounding_box(&s_min, &s_max);
		scene_min = glm::make_vec3((float*)&s_min);
		scene_max = glm::make_vec3((float*)&s_max);
		scene_center  = (scene_min+scene_max)/2.0f;

		//Generate vertexarrayobjects
		pre_render();
	} else {
		printf("Failed to load model %s\n", model.c_str());
	}
}

void RenderObject::pre_render() {

	glUseProgram(shader.program);

	recursive_pre_render(scene->mRootNode);

	glUseProgram(0);
}

void RenderObject::recursive_pre_render(const aiNode* node) {
	GLuint vao[node->mNumMeshes];
	glGenVertexArrays(node->mNumMeshes, vao);

	printf("%s\n", node->mName.data);

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		glBindVertexArray(vao[i]);
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		
		float bufferData[mesh->mNumVertices*2*3]; //Vertices and normals
		
		unsigned short numIndices = mesh->mFaces[0].mNumIndices;
		if(numIndices > 4) {
			printf("No support for gt quads\n");
			exit(-1);
		}	
		unsigned int indexData[mesh->mNumFaces*3];

		for(unsigned int n = 0; n<mesh->mNumVertices; ++n) {
			memcpy((void*)(bufferData + n*3), &mesh->mVertices[n].x, 3*sizeof(float));
			if(mesh->HasNormals())
				memcpy((void*)(bufferData + mesh->mNumVertices*3 + n*3), &mesh->mNormals[n].x, 3*sizeof(float));
		}

		for(unsigned int n=0; n<mesh->mNumFaces; ++n) {
			if(mesh->mFaces[n].mNumIndices != numIndices) {
				printf("Only supports meshes with same numIndies on faces!\n");
				exit(-1);
			}
			printf("(%f, %f, %f)\n(%f, %f, %f)\n(%f, %f, %f)\n\n", 
				bufferData[mesh->mFaces[n].mIndices[0]*3],
				bufferData[mesh->mFaces[n].mIndices[0]*3+1],
				bufferData[mesh->mFaces[n].mIndices[0]*3+2],
				bufferData[mesh->mFaces[n].mIndices[1]*3],
				bufferData[mesh->mFaces[n].mIndices[1]*3+1],
				bufferData[mesh->mFaces[n].mIndices[1]*3+2],
				bufferData[mesh->mFaces[n].mIndices[2]*3],
				bufferData[mesh->mFaces[n].mIndices[2]*3+1],
				bufferData[mesh->mFaces[n].mIndices[2]*3+2]);
			memcpy((void*)(indexData+n*numIndices),mesh->mFaces[n].mIndices, numIndices*sizeof(unsigned int));
		}

		GLuint buffers[2];
		glGenBuffers(2, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		//TODO: Normals
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
		
		glBindVertexArray(0);

		mesh_buffers[mesh] = buffers;
	}

	node_vao[node] = vao;


	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_pre_render(node->mChildren[i]);
	}
}

void RenderObject::recursive_render(const aiNode* node, double dt) {
	modelViewMatrix.Push();

	aiMatrix4x4 m = node->mTransformation; 	
	aiTransposeMatrix4(&m);
	modelViewMatrix *= glm::make_mat4((float*)&m);

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		glBindVertexArray(node_vao[node][i]);
		const aiMesh* mesh = scene->mMeshes[i];

		glBindBuffer(GL_ARRAY_BUFFER, mesh_buffers[mesh][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_buffers[mesh][1]);
		GLenum draw_mode;
		switch(mesh->mFaces[0].mNumIndices) {
			case 1:
				draw_mode = GL_POINTS;
				break;
			case 2:
				draw_mode = GL_LINES;
				break;
			case 3:
				draw_mode = GL_TRIANGLES;
				break;
			case 4:
				draw_mode = GL_QUADS;
				break;
		}

		glUniformMatrix4fv(shader.mvp, 1, GL_FALSE, glm::value_ptr(modelViewMatrix.Top()));

		glDrawElements(draw_mode, mesh->mNumFaces, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_render(node->mChildren[i], dt);
	}

	modelViewMatrix.Pop();
}

void RenderObject::render(double dt) {

	modelViewMatrix.Push();

	float tmp = scene_max.x-scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y,tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z,tmp);
	tmp = 1.f / tmp;

	modelViewMatrix.Scale(tmp);

	modelViewMatrix.Translate( -scene_center.x, -scene_center.y, 10.0f );
	//modelViewMatrix.Translate( 0.5, 0.7, 10.0);

	recursive_render(scene->mRootNode, dt);		

	modelViewMatrix.Pop();
}


void RenderObject::get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo){

	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;
	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {
			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}



void RenderObject::get_bounding_box (struct aiVector3D* min, struct aiVector3D* max) {
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

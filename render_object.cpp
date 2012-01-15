#include "render_object.h"
#include "render.h"
#include <string>
#include <cstdio>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <glm/glm.hpp>
#include <glimg/glimg.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

RenderObject::RenderObject(std::string model) {
	scene = aiImportFile( model.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals );
	rotation = 0;
	if(scene != 0) {
		printf("Loaded model %s: \nMeshes: %d\nTextures: %d\nMaterials: %d\n",model.c_str(), scene->mNumMeshes, scene->mNumTextures, scene->mNumMaterials);

		//Get bounds:
		aiVector3D s_min, s_max;
		get_bounding_box(&s_min, &s_max);
		scene_min = glm::make_vec3((float*)&s_min);
		scene_max = glm::make_vec3((float*)&s_max);
		scene_center  = (scene_min+scene_max)/2.0f;

		pre_render();
	} else {
		printf("Failed to load model %s\n", model.c_str());
	}
}

void RenderObject::pre_render() {

	glUseProgram(shader.program);

	recursive_pre_render(scene->mRootNode);

	//Init materials:
	for(unsigned int i= 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial * mtl = scene->mMaterials[i];
		material_t mtl_data;
		aiString path;
		std::string full_path;
		if(mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0 && 
			mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			full_path = std::string("textures/")+p;
		} else {
			full_path = "textures/notex.png";
		}
		printf("Load texture %s\n", full_path.c_str());
		glimg::ImageSet *pImgSet = glimg::loaders::stb::LoadFromFile(full_path.c_str());
		mtl_data.texture = glimg::CreateTexture(pImgSet, 0);
		delete pImgSet;
		materials.push_back(mtl_data);
	}

	glUseProgram(0);
}

void RenderObject::recursive_pre_render(const aiNode* node) {
	const aiVector3D zero_3d(0.0f,0.0f,0.0f);
	
	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mesh_data_t md;

		md.mtl_index = mesh->mMaterialIndex;

		std::vector<vertex_t> vertexData;
		std::vector<unsigned int> indexData;

		for(unsigned int n = 0; n<mesh->mNumVertices; ++n) {
			const aiVector3D* pos = &(mesh->mVertices[n]);
			const aiVector3D* texCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][n]) : &zero_3d;
			const aiVector3D* normal = &(mesh->mNormals[n]);
			vertexData.push_back(vertex_t(pos, texCoord, normal));
		}

		for(unsigned int n = 0 ; n<mesh->mNumFaces; ++n) {
			const aiFace* face = &mesh->mFaces[n];
			assert(face->mNumIndices == 3);
			md.num_indices+=3;

			for(unsigned int j = 0; j< face->mNumIndices; ++j) {
				int index = face->mIndices[j];
				indexData.push_back(index);
			}
		}

		glGenBuffers(1, &md.vb);

		glBindBuffer(GL_ARRAY_BUFFER, md.vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*vertexData.size(), &vertexData.front(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &md.ib);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md.ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexData.size(), &indexData.front(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh_data[mesh] = md;
	}

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_pre_render(node->mChildren[i]);
	}
}

void RenderObject::recursive_render(const aiNode* node, double dt) {
	modelViewMatrix.Push();

	aiMatrix4x4 m = node->mTransformation; 	
	aiTransposeMatrix4(&m);
	modelViewMatrix *= glm::make_mat4((float*)&m);
	glUniformMatrix4fv(shader.mvp, 1, GL_FALSE, glm::value_ptr(modelViewMatrix.Top()));

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if(mesh->mNumFaces > 0) {
			mesh_data_t *md = &mesh_data[mesh];

			glBindBuffer(GL_ARRAY_BUFFER, md->vb);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md->ib);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(float)*3));

			glBindTexture(GL_TEXTURE_2D, materials[md->mtl_index].texture);

			glDrawElements(GL_TRIANGLES, md->num_indices, GL_UNSIGNED_INT,0 );

			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
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



	modelViewMatrix.Translate( -scene_center.x, -scene_center.y, -scene_center.z );
	modelViewMatrix.Scale(tmp);
	modelViewMatrix.Scale(2);
	rotation += 20.0f*dt;

	rotation = fmod(rotation, 360.0f);
	
	modelViewMatrix.RotateY(rotation);
	modelViewMatrix.RotateZ(rotation);
	modelViewMatrix.RotateX(rotation);

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

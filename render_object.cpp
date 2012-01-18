#include "render_object.h"
#include "render_group.h"
#include "renderer.h"
#include <string>
#include <cstdio>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <glm/glm.hpp>
#include <glimg/glimg.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

void RenderObject::color4_to_vec4(const struct aiColor4D *c, glm::vec4 &target) {
	target.x = c->r;
	target.y = c->g;
	target.z = c->b;
	target.w = c->a;
}

RenderObject::~RenderObject() {
	aiReleaseImport(scene);
}

RenderObject::RenderObject(std::string model, Renderer::shader_program_t shader_program, bool normalize_scale, unsigned int aiOptions) 
	: RenderGroup(), shader_program_(shader_program) {

	scene = aiImportFile( model.c_str(), 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |  aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  |
		aiProcess_FixInfacingNormals | aiOptions
		);

	if(scene != 0) {
		printf("Loaded model %s: \nMeshes: %d\nTextures: %d\nMaterials: %d\n",model.c_str(), scene->mNumMeshes, scene->mNumTextures, scene->mNumMaterials);

		//Get bounds:
		aiVector3D s_min, s_max;
		get_bounding_box(&s_min, &s_max);
		scene_min = glm::make_vec3((float*)&s_min);
		scene_max = glm::make_vec3((float*)&s_max);
		scene_center  = (scene_min+scene_max)/2.0f;

		//Calculate normalization matrix
		glutil::MatrixStack normMatrix;
		if(normalize_scale) {
			glm::vec3 size = scene_max - scene_min;
			float tmp = aisgl_max(size.x, size.y);
			tmp = aisgl_max(tmp, size.z);
			normMatrix.Scale(1.f/tmp);
		}
		normMatrix.Translate(-scene_center.x, -scene_center.y, -scene_center.z);
		
		normalization_matrix_ = normMatrix.Top();

		pre_render();


	} else {
		printf("Failed to load model %s\n", model.c_str());
	}
}

void RenderObject::pre_render() {

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
			size_t last_slash = p.rfind("/");
			if(last_slash != std::string::npos) 
				p = p.substr(last_slash+1);
			full_path = std::string("textures/")+p;
			printf("Load texture %s\n", full_path.c_str());
			glimg::ImageSet *pImgSet = glimg::loaders::stb::LoadFromFile(full_path.c_str());
			mtl_data.texture = glimg::CreateTexture(pImgSet, 0);
			delete pImgSet;
			mtl_data.attr.extra= 1;//toggle textures on
		} else {
			mtl_data.attr.extra= 0; //toggle textures off
		}

		aiString name;
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_NAME, name))
			printf("Loaded material [%d] %s\n", i, name.data);

		aiColor4D diffuse;
		aiColor4D specular;
		aiColor4D ambient;
		aiColor4D emission;	
		mtl_data.attr.diffuse = glm::vec4( 0.8f, 0.8f, 0.8f, 1.0f);
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			color4_to_vec4(&diffuse, mtl_data.attr.diffuse);

		mtl_data.attr.specular = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f);
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
			color4_to_vec4(&specular, mtl_data.attr.specular);

		mtl_data.attr.ambient = glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f);
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
			color4_to_vec4(&ambient, mtl_data.attr.ambient);

		mtl_data.attr.emission = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f);
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
			color4_to_vec4(&emission, mtl_data.attr.emission);

		unsigned int max = 1;
		float strength;
		int ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &mtl_data.attr.shininess, &max);
		if(ret1 == AI_SUCCESS) {
			max = 1;
			int ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
			if(ret2 == AI_SUCCESS)
				mtl_data.attr.shininess *= strength;
		} else {
			mtl_data.attr.shininess = 0.0f;
			mtl_data.attr.specular = glm::vec4(0.f, 0.f, 0.f, 0.f);
		}
		max = 1;
		int two_sided;
		if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
			mtl_data.two_sided = true;

		materials.push_back(mtl_data);
	}

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
			if(!mesh->HasNormals())
				normal = &zero_3d;
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

void RenderObject::recursive_render(const aiNode* node, double dt, Renderer * renderer) {
	renderer->modelMatrix.Push();

	aiMatrix4x4 m = node->mTransformation; 	
	aiTransposeMatrix4(&m);
	renderer->modelMatrix *= glm::make_mat4((float*)&m);

	renderer->upload_model_matrices();

	for(unsigned int i=0; i<node->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if(mesh->mNumFaces > 0) {
			mesh_data_t *md = &mesh_data[mesh];

			glBindBuffer(GL_ARRAY_BUFFER, md->vb);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md->ib);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(float)*3));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(float)*5));

			materials[md->mtl_index].activate(renderer);

			glDrawElements(GL_TRIANGLES, md->num_indices, GL_UNSIGNED_INT,0 );

			materials[md->mtl_index].deactivate();

			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_render(node->mChildren[i], dt, renderer);
	}

	renderer->modelMatrix.Pop();
}

void RenderObject::render(double dt, Renderer * renderer) {

	glUseProgram(renderer->shaders[shader_program_].program);

	renderer->modelMatrix.Push();
	renderer->modelMatrix.ApplyMatrix(matrix());

	recursive_render(scene->mRootNode, dt, renderer);		

	renderer->modelMatrix.Pop();
}

const glm::mat4 RenderObject::matrix() const {
	return RenderGroup::matrix() * normalization_matrix_;
}

void RenderObject::material_t::activate(Renderer * renderer) {
	if(two_sided)
		glDisable(GL_CULL_FACE);

	glBindTexture(GL_TEXTURE_2D, texture);

	//Upload material attributes to shader
	glBindBuffer(GL_UNIFORM_BUFFER, renderer->shader_globals.materialBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer::shader_material_t), &attr);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void RenderObject::material_t::deactivate() {
	if(two_sided)
		glEnable(GL_CULL_FACE);
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

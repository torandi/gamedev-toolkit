#include "render_object.h"
#include "render_group.h"
#include "renderer.h"
#include "texture.h"
#include <string>
#include <cstdio>
#include <algorithm>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

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
	for(std::map<const aiMesh*, mesh_data_t>::iterator it=mesh_data.begin(); it!=mesh_data.end(); ++it) {
		glDeleteBuffers(1, &it->second.vb);
		glDeleteBuffers(1, &it->second.ib);
	}

	for(std::vector<material_t>::iterator it=materials.begin(); it!=materials.end(); ++it) {
		if(it->texture != NULL)
			delete it->texture;
		if(it->normal_map != NULL)
			delete it->normal_map;
	}
}

RenderObject::RenderObject(std::string model, Renderer::shader_program_t shader_program, bool normalize_scale, unsigned int aiOptions) 
	: RenderGroup(), shader_program_(shader_program) {

	name = model;
	current_animation_ = -1;
	run_animation_ = false;
	current_frame_ = 0;
	loop_back_frame_ = 0;

	scene = aiImportFile( model.c_str(), 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |  
		aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  |
		aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords |
		aiProcess_ValidateDataStructure | aiProcess_FixInfacingNormals |
		aiProcess_CalcTangentSpace | aiOptions
		);

	if(scene != 0) {
		printf("Loaded model %s: \nMeshes: %d\nTextures: %d\nMaterials: %d\nAnimations: %d\n",model.c_str(), scene->mNumMeshes, scene->mNumTextures, scene->mNumMaterials, scene->mNumAnimations);

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
/*
		if(scene->HasAnimations()) {
			printf("Animation data:\n");
			for(unsigned int i=0; i < scene->mNumAnimations; ++i) {
				aiAnimation * anim = scene->mAnimations[i];
				printf("Name: %s, Bone Channels: %d, Mesh channels: %d, tps: %f, duration: %f\n", anim->mName.data, anim->mNumChannels, anim->mNumMeshChannels, anim->mTicksPerSecond, anim->mDuration);
				for(unsigned int n=0; n<anim->mNumChannels; ++n) {
					aiNodeAnim * na = anim->mChannels[n];
					printf("nodeAnim: Node name: %s, position keys: %d, rotation keys: %d, scaling keys: %d\n", na->mNodeName.data, na->mNumPositionKeys, na->mNumRotationKeys, na->mNumScalingKeys);
				}
			}
		}
*/
	} else {
		printf("Failed to load model %s\n", model.c_str());
	}
}

void RenderObject::run_animation(double dt) {
	if(current_animation_ != -1) {
		double tps = scene->mAnimations[current_animation_]->mTicksPerSecond;
		tps = (tps == 0) ? 50.0 : tps;
		current_frame_ += tps*dt;
		if(current_frame_ >= end_frame_) {
			switch(anim_end_behaviour_) {
				case ANIM_LOOP:
					current_frame_ = loop_back_frame_;
					break;
				case ANIM_STOP_AT_END:
					current_frame_ = end_frame_;
					run_animation_ = false;
					break;
				case ANIM_STOP_AT_START:
					current_frame_ = loop_back_frame_;
					run_animation_ = false;
					break;
				case ANIM_STOP_AT_END_AND_GO_TO_POST_FRAME:
					current_frame_ = post_frame_;
					run_animation_ = false;
					break;
				case ANIM_RESET:
					current_frame_ = 0;
					run_animation_ = false;
					current_animation_ = -1;
					break;
			}	
		}	
	} else {
		fprintf(stderr, "Error! Running animation with current_animation_ == -1! in model %s\n", name.c_str());
	}
}

bool RenderObject::start_animation(unsigned int anim, double start_frame, double end_frame, anim_end_behaviour_t end_behaviour) {
	if(!scene->HasAnimations() || anim >= scene->mNumAnimations) 
		return false;
	current_animation_ = anim;
	run_animation_ = true;
	if(start_frame != -1) {
		current_frame_ = start_frame;
		loop_back_frame_ = start_frame;
	}
	if(end_frame == -1)
		end_frame_ = scene->mAnimations[current_animation_]->mDuration;
	else
		end_frame_ = end_frame;

	anim_end_behaviour_ = end_behaviour;	
	return true;
}

bool RenderObject::stop_animation(double end_frame, double stop_frame) {
	if(!run_animation_)
		return false;
	if(end_frame == -1) {
		//Stop now
		run_animation_ = false;
		if(stop_frame > -1)
			current_frame_ = stop_frame;
		else if(stop_frame == -1)
			current_animation_ = -1;
	} else {
		end_frame_ = end_frame;
		if(stop_frame > -1) {
			post_frame_ = stop_frame;
			anim_end_behaviour_ = ANIM_STOP_AT_END_AND_GO_TO_POST_FRAME;
		} else if(stop_frame == -1) {
			anim_end_behaviour_ = ANIM_RESET;
		} else {
			anim_end_behaviour_ = ANIM_STOP_AT_END;
		}
	}
	return true;
}

Texture * RenderObject::load_texture(std::string path) {
	size_t last_slash = path.rfind("/");
	if(last_slash != std::string::npos) 
		path = path.substr(last_slash+1);
	std::string full_path = std::string("textures/")+path;
	return new Texture(full_path);
}

void RenderObject::pre_render() {

	recursive_pre_render(scene->mRootNode);


	//Init materials:
	for(unsigned int i= 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial * mtl = scene->mMaterials[i];
		material_t mtl_data;
		aiString path;
		if(mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0 && 
			mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			mtl_data.texture = load_texture(p);
			mtl_data.attr.use_texture = 1;//toggle textures on
		} else if(mtl->GetTextureCount(aiTextureType_AMBIENT) > 0 && 
			mtl->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			mtl_data.texture = load_texture(p);
			mtl_data.attr.use_texture = 1;//toggle textures on
		} else {
			mtl_data.attr.use_texture = 0; //toggle textures off
		}
	
		//Check for normalmap:
		if(mtl->GetTextureCount(aiTextureType_HEIGHT) > 0 && 
			mtl->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string p(path.data);
			mtl_data.normal_map = load_texture(p);
			mtl_data.attr.use_normal_map = 1;
			printf("Using normal map: %s\n", p.c_str());
		} else {
			mtl_data.attr.use_normal_map = 0;
		}

		aiString name;
		mtl->Get(AI_MATKEY_NAME, name);

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

	node_data_t nd;
	nd.name = node->mName;
	//Find animation:

	if(scene->HasAnimations()) {
		nd.animations = new aiNodeAnim*[scene->mNumAnimations];
		for(unsigned int i=0; i < scene->mNumAnimations; ++i) {
			nd.animations[i] = NULL;
			aiAnimation * anim = scene->mAnimations[i];
			for(unsigned int n=0; n<anim->mNumChannels; ++n) {
				aiNodeAnim * na = anim->mChannels[n];
				if(na->mNodeName == node->mName) {
					nd.animations[i] = na;
					break;
				}
			}

		}
	} else {
		nd.animations = NULL;
	}
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
			const aiVector3D* tangent, *bitangent;
			if(mesh->HasTangentsAndBitangents()) {
				tangent = &(mesh->mTangents[n]);
				bitangent= &(mesh->mBitangents[n]);
			} else {
				tangent = &zero_3d;
				bitangent = &zero_3d;
			}
			if(!mesh->HasNormals())
				normal = &zero_3d;
			vertexData.push_back(vertex_t(pos, texCoord, normal, tangent, bitangent));
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

	node_data_[node] = nd;

	for(unsigned int i=0; i<node->mNumChildren; ++i) {
		recursive_pre_render(node->mChildren[i]);
	}
}

void RenderObject::recursive_render(const aiNode* node, double dt, Renderer * renderer) {
	renderer->modelMatrix.Push();
	
	node_data_t nd = node_data_[node];

	//Run animation or apply default transform
	if(run_animation_ && nd.animations !=NULL && nd.animations[current_animation_] != NULL) {
		aiNodeAnim * na = nd.animations[current_animation_];
		int next_keyframe;
	
		//Translation:
			//Find next keyframe:
			next_keyframe = na->mNumPositionKeys-1;
			for(unsigned int i=0; i< na->mNumPositionKeys; ++i) {
				if(na->mPositionKeys[i].mTime > current_frame_) {
					next_keyframe = i;
					break;
				}
			}
			glm::vec3 translation; //Translation to apply
			if(next_keyframe == 0) {
				//no keyframe before this one
				translation = glm::make_vec3((float*)&na->mPositionKeys[next_keyframe].mValue);
			} else {
				glm::vec3 prev, next;
				float blend; 
				aiVectorKey &k_prev = na->mPositionKeys[next_keyframe-1];
				aiVectorKey &k_next = na->mPositionKeys[next_keyframe];
				prev = glm::make_vec3((float*)&k_prev.mValue);
				next = glm::make_vec3((float*)&k_next.mValue);
				float interval = k_next.mTime - k_prev.mTime;
				float pos = current_frame_ - k_prev.mTime;
				blend = pos/interval;
				blend = std::min(blend, 1.f);
				translation = glm::mix(prev, next, blend);
			}
			renderer->modelMatrix.Translate(translation);

		//Rotation
			//Find next keyframe:
			next_keyframe = na->mNumRotationKeys-1;
			for(unsigned int i=0; i<na->mNumRotationKeys; ++i) {
				if(na->mRotationKeys[i].mTime > current_frame_) {
					next_keyframe = i;
					break;
				}
			}
			aiQuaternion rotation; //Rotation to apply
			if(next_keyframe == 0) {
				//no keyframe before this one
				rotation = na->mRotationKeys[next_keyframe].mValue;
			} else {
				float blend; 
				aiQuatKey &k_prev = na->mRotationKeys[next_keyframe-1];
				aiQuatKey &k_next = na->mRotationKeys[next_keyframe];
				float interval = k_next.mTime - k_prev.mTime;
				float pos = current_frame_ - k_prev.mTime;
				blend = pos/interval;
				blend = std::min(blend, 1.f);
				aiQuaternion::Interpolate(rotation, k_prev.mValue, k_next.mValue, blend); 
			}
			glm::fquat q;
			q.x = rotation.x;
			q.y = rotation.y;
			q.z = rotation.z;
			q.w = rotation.w;
			renderer->modelMatrix *= glm::mat4_cast(q);

		//Scaling
			//Find next keyframe:
			next_keyframe = na->mNumScalingKeys-1;
			for(unsigned int i=0; i<na->mNumScalingKeys; ++i) {
				if(na->mScalingKeys[i].mTime > current_frame_) {
					next_keyframe = i;
					break;
				}
			}
			glm::vec3 scaling; //scaling to apply
			if(next_keyframe == 0) {
				//no keyframe before this one
				scaling = glm::make_vec3((float*)&na->mScalingKeys[next_keyframe].mValue);
			} else {
				glm::vec3 prev, next;
				float blend; 
				aiVectorKey & k_prev = na->mScalingKeys[next_keyframe-1];
				aiVectorKey & k_next = na->mScalingKeys[next_keyframe];
				prev = glm::make_vec3((float*)&k_prev.mValue);
				next = glm::make_vec3((float*)&k_next.mValue);
				float interval = k_next.mTime - k_prev.mTime;
				float pos = current_frame_ - k_prev.mTime;
				blend = pos/interval;
				blend = std::min(blend, 1.f);
				scaling = glm::mix(prev, next, blend);
			}
			renderer->modelMatrix.Scale(scaling);
	} else {
		aiMatrix4x4 m = node->mTransformation; 	
		aiTransposeMatrix4(&m);
		renderer->modelMatrix *= glm::make_mat4((float*)&m);
	}

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
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)+sizeof(glm::vec2)));
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (2*sizeof(glm::vec3)+sizeof(glm::vec2)));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (3*sizeof(glm::vec3)+sizeof(glm::vec2)));

			materials[md->mtl_index].activate(renderer);
			Renderer::checkForGLErrors("RenderObject::activate material");

			glDrawElements(GL_TRIANGLES, md->num_indices, GL_UNSIGNED_INT,0 );
			Renderer::checkForGLErrors("RenderObject::render()");

			materials[md->mtl_index].deactivate(renderer);

			glDisableVertexAttribArray(4);
			glDisableVertexAttribArray(3);
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

	if(run_animation_)
		run_animation(dt);

	glUseProgram(renderer->shaders[shader_program_].program);

	renderer->modelMatrix.Push();
	renderer->modelMatrix.ApplyMatrix(matrix());

	recursive_render(scene->mRootNode, dt, renderer);		

	renderer->modelMatrix.Pop();

	glUseProgram(0);
}

const glm::mat4 RenderObject::matrix() const {
	return RenderGroup::matrix() * normalization_matrix_;
}

void RenderObject::material_t::activate(Renderer * renderer) {
	if(two_sided && renderer->cull_face)
		glDisable(GL_CULL_FACE);
	

	if(attr.use_texture) {
		glActiveTexture(GL_TEXTURE0);
		texture->bind();
	}
	
	if(attr.use_normal_map) {
		glActiveTexture(GL_TEXTURE1);
		normal_map->bind();
	}

	//Upload material attributes to shader
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.materialBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Shader::material_t), &attr);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void RenderObject::material_t::deactivate(Renderer * renderer) {
	if(two_sided && renderer->cull_face)
		glEnable(GL_CULL_FACE);

	if(attr.use_texture) {
		glActiveTexture(GL_TEXTURE0);
		texture->unbind();
	}
	
	if(attr.use_normal_map) {
		glActiveTexture(GL_TEXTURE1);
		normal_map->unbind();
	}
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

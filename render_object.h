#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <string>
#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <map>
#include <vector>
#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <glutil/glutil.h>

#include "renderer.h"
#include "render_group.h"
#include "texture.h"

class RenderObject : public RenderGroup {
public:
	enum anim_end_behaviour_t {
		ANIM_LOOP,
		ANIM_STOP_AT_END, //stop at end frame
		ANIM_STOP_AT_START, //stop at start frame (after first loop)
		ANIM_STOP_AT_END_AND_GO_TO_POST_FRAME, //stop at post_frame_ (set by stop_animation)
		ANIM_RESET //After end frame, stop and return to default look
	};
private:
	glm::mat4 normalization_matrix_;

	void get_bounding_box_for_node (const struct aiNode* nd,	struct aiVector3D* min, struct aiVector3D* max, struct aiMatrix4x4* trafo);
	void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
	void color4_to_vec4(const struct aiColor4D *c, glm::vec4 &target);

	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	Renderer::shader_program_t shader_program_;

	//Trims path and loads texture
	static Texture * load_texture(std::string path);

	double current_frame_;
	int current_animation_;
	bool run_animation_;
	double loop_back_frame_;
	double post_frame_; //Frame to stop at if ANIM_STOP_AT_POST_FRAME is specified
	double end_frame_;
	anim_end_behaviour_t anim_end_behaviour_;
	
	struct node_data_t {
		aiNodeAnim ** animations;
		aiString name;
	};

	std::map<const aiNode*, node_data_t> node_data_;

	//Updates the current_frame and other animation statuses
	void run_animation(double dt);

public:
	const aiScene* scene;
	glm::vec3 scene_min, scene_max, scene_center;
	std::string name;
	double anim_speed;

	struct mesh_data_t {
		mesh_data_t() : num_indices(0) {};
		GLuint vb, ib; //vertex buffer, index buffer
		GLenum draw_mode;
		unsigned int num_indices;
		unsigned int mtl_index;
	};



	struct vertex_t {
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		vertex_t(const aiVector3D* pos_,const  aiVector3D* texCoord_,
					const aiVector3D* normal_, const aiVector3D * tangent_, const aiVector3D * bitangent_) {
			pos = glm::vec3(pos_->x,pos_->y, pos_->z);
			texCoord = glm::vec2(texCoord_->x,texCoord_->y);
			normal = glm::vec3(normal_->x, normal_->y, normal_->z);
			tangent = glm::vec3(tangent_->x, tangent_->y, tangent_->z);
			bitangent = glm::vec3(bitangent_->x, bitangent_->y, bitangent_->z);
		}
	};

	struct material_t {
		material_t() : two_sided(false), texture(NULL), normal_map(NULL) {};
		Shader::material_t attr;
		bool two_sided;
		Texture * texture, *normal_map;

		void activate(Renderer * renderer);
		void deactivate(Renderer * renderer);
	};

	//Set normalize_scale to false to not scale down to 1.0
	RenderObject(std::string model, Renderer::shader_program_t shader_program, bool normalize_scale=true, unsigned int aiOptions=0);
	virtual ~RenderObject();

	std::vector<material_t> materials;

	std::map<const aiMesh*, mesh_data_t > mesh_data;

	/*
	 * If start_frame is -1 the animation will start from the current frame (0 from the begining)
	 * The frame to loop back to is not changed (0 from the begining)
	 * If end_frame is -1 end_frame will be set to animation durration.
	 */
	bool start_animation(unsigned int anim=0, double start_frame=-1, double end_frame=-1, anim_end_behaviour_t end_behaviour=ANIM_RESET);
	/*
	 * Set endframe to != -1 to wait to given frame until stop, otherwise it will stop instantly
	 * Set set_frame to set that frame now (or after end_frame). Set set_frame to -1 to return to non animation behaviour
	 * Leave set_frame at -2 to stop at end_frame
	 * Returns true if an anmiation was running
	*/
	bool stop_animation(double end_frame=-1, double set_frame=-2);
	float current_frame() { return current_frame_; };
	bool is_animating() { return run_animation_; };

	void pre_render();
	void recursive_pre_render(const aiNode* node);
	void recursive_render(const aiNode* node, double dt, Renderer * renderer);
	virtual void render(double dt, Renderer * renderer);
	virtual const glm::mat4 matrix() const;

	
};

#endif

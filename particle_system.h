#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include "render_group.h"
#include "renderer.h"
#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <string>
#include <list>

#define MAX_NUM_PARTICLES 500

class ParticleSystem : public RenderGroup {
	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	RenderObject * cube_;

	//Private vars
	glm::vec3 spawn_area_;
	float regen_, avg_ttl_, ttl_var_, avg_spawn_speed_, spawn_speed_var_, avg_acc_, acc_var_, avg_deacc_, deacc_var_;
	Texture * texture_;
	glm::vec4 color1_, color2_;
	glm::vec3 spawn_direction_, direction_var_;
	float avg_scale_, scale_var_;

	GLuint ib_;
	GLuint vb_;

	Renderer::shader_program_t shader_;	

	float particle_rest_;

	struct vertex_t {
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec4 color;
	};

	struct particle_t {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 direction;
		float ttl;
		float speed;
		float acc;
		float deacc;
		float scale;

		//Returns false if the particle died
		bool update(double dt);
		//v is a pointer to the first vertex of this particle
		void update_vertices(vertex_t *v);
	};

	std::list<particle_t> particles_;
	vertex_t * vertices_;

	void generate_buffers();

	float rand(float var, bool d=true); //d=true -> double sided, => 2*var*frand()-var
	glm::vec3 rand(glm::vec3 var, bool d=true);

	public:
	/*
	 * Position: coordinate of the lower left corner of the spawn area
	 * spawn_area_size: Size of spawn area
	 * regeneration: new particles/second
	 * avg_ttl: averange ttl
	 * ttl_var: ttl variance
	 * shader: shader to use
	 * texture: texture to load
	 * color1: start color
	 * color2: end color, color will be randomized in span color1-color2
	 */
	ParticleSystem(
		glm::vec3 position, glm::vec3 spawn_area_size, float regeneration, float avg_ttl, float ttl_var,
		float avg_spawn_speed,float  spawn_speed_var,float  avg_acc,float  acc_var,float  avg_deacc,float  deacc_var,
		glm::vec3 spawn_direction, glm::vec3 direction_var, float avg_scale, float scale_var,
		Renderer::shader_program_t shader, std::string texture, glm::vec4 color1, glm::vec4 color2
	);
	~ParticleSystem();

	void spawn_particles(int num_particles);

	void update(double dt);
	void render(double dt, Renderer * renderer);
};

#endif

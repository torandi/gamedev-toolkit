#include <glm/glm.hpp>
#include <glload/gl_3_3.h>
#include <string>
#include <cstdlib>

#include "particle_system.h"
#include "util.h"
#include "render_object.h"

#define NUM_SIDES 2
#define VERTICES_PER_SIDE 4
#define INDICES_PER_SIDE 6
#define VERTICES_PER_PARTICLE NUM_SIDES*VERTICES_PER_SIDE
#define INDICES_PER_PARTICLE NUM_SIDES*INDICES_PER_SIDE

bool ParticleSystem::particle_t::update(double dt) {
	ttl-=dt;
	if(ttl<=0.0)
		return false;
	
	acc-=deacc*dt;
	speed+=acc*dt;
	position+=(direction*(float)(speed*dt))+rand(motion_randomization*(float)dt);
	//TODO: Change color during lifespan, and maybe alpha too

	return true;
}

void ParticleSystem::particle_t::update_vertices(ParticleSystem::vertex_t * v) {
	//Update attributes that are the same for all vertices:
	for(int n=0; n<4*NUM_SIDES;++n) {
		v[n].color = color;
	}

	//Update position:
	v[0].position = position+glm::vec3(-0.5, -0.5, 0)*scale;
	v[1].position = position+glm::vec3(0.5, -0.5, 0)*scale;
	v[2].position = position+glm::vec3(-0.5, 0.5, 0)*scale;
	v[3].position = position+glm::vec3(0.5, 0.5, 0)*scale;

	v[4].position = position+glm::vec3(0, -0.5, -0.5)*scale;
	v[5].position = position+glm::vec3(0, 0.5, -0.5)*scale;
	v[6].position = position+glm::vec3(0, -0.5, 0.5)*scale;
	v[7].position = position+glm::vec3(0, 0.5, 0.5)*scale;
}

ParticleSystem::~ParticleSystem() {
	delete[] vertices_;
	delete texture_;
}	

ParticleSystem::ParticleSystem(
	glm::vec3 position, glm::vec3 spawn_area, float regeneration, float avg_ttl, float ttl_var,
	float avg_spawn_speed,float  spawn_speed_var,float  avg_acc,float  acc_var,float  avg_deacc,float  deacc_var,
	glm::vec3 spawn_direction, glm::vec3 direction_var, float avg_scale, float scale_var,
	Renderer::shader_program_t shader, std::string texture, glm::vec4 color1, glm::vec4 color2, glm::vec3 motion_rand
) : RenderGroup(position), 
	spawn_area_(spawn_area), regen_(regeneration), avg_ttl_(avg_ttl), ttl_var_(ttl_var),
	avg_spawn_speed_(avg_spawn_speed), spawn_speed_var_(spawn_speed_var), avg_acc_(avg_acc), acc_var_(acc_var),
	avg_deacc_(avg_deacc), deacc_var_(deacc_var), color1_(color1), color2_(color2), motion_rand_(motion_rand),
	spawn_direction_(spawn_direction), direction_var_(direction_var), 
	avg_scale_(avg_scale), scale_var_(scale_var),	
	shader_(shader), particle_rest_(0.f)
{
	cube_ = new RenderObject("models/cube.obj", Renderer::DEBUG_SHADER);
	cube_->scale = spawn_area;
	texture_ = new Texture(texture);
	vertices_ = new vertex_t[MAX_NUM_PARTICLES*NUM_SIDES*4];
	generate_buffers();
	enabled = true;
}

/*
 * Winding order: CCW
 * 
	One side:
 	3-----4
	|		|
	1 --- 2
	Drawing order: 1, 2, 4; 1, 4, 3
 */

void ParticleSystem::generate_buffers() {
	std::vector<unsigned int> indices;
	//The vertices buffer is filled as needed while the indices buffer is filled once, the texCoord is static in the vb though
	for(int i=0; i<MAX_NUM_PARTICLES; ++i) {
		for(int n=0;n<NUM_SIDES; ++n) {
			int base_index = (i*NUM_SIDES*4)+n*4;
			//Set texture coordinates on the verticel
			vertices_[base_index+0].texCoord = glm::vec2(0,0);
			vertices_[base_index+1].texCoord = glm::vec2(1,0);
			vertices_[base_index+2].texCoord = glm::vec2(0,1);
			vertices_[base_index+3].texCoord = glm::vec2(1,1);

			//indices:
			//Face 1
			indices.push_back(base_index+0);
			indices.push_back(base_index+1);
			indices.push_back(base_index+3);
			//Face 2
			indices.push_back(base_index+0);
			indices.push_back(base_index+3);
			indices.push_back(base_index+2);
		}
	}
	//Generate buffers and upload:
	glGenBuffers(1, &vb_);
	
	glBindBuffer(GL_ARRAY_BUFFER,vb_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*MAX_NUM_PARTICLES*NUM_SIDES*4, NULL, GL_DYNAMIC_DRAW);
	Renderer::checkForGLErrors("ParticleSystem::generate_buffers() - buffer vertices");
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ib_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ib_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*MAX_NUM_PARTICLES*NUM_SIDES*6, &indices.front(), GL_STATIC_DRAW);
	Renderer::checkForGLErrors("ParticleSystem::generate_buffers() - buffer indices");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	assert(vb_ >0 && ib_>0);
}

void ParticleSystem::spawn_particles(int num_particles) {
	srand(time(NULL));
	for(int i=0;i<num_particles;++i) {
		seed_random();
		particle_t p;
		p.position = position_ + rand(spawn_area_, false);
		float m = frand();
		p.color = (1-m)*color1_  + m*color2_;
		p.direction = glm::normalize(spawn_direction_ + rand(direction_var_));
		if(p.direction.length() == 0.0)
			p.direction = glm::normalize(spawn_direction_);
		p.ttl = avg_ttl_ + rand(ttl_var_);
		p.speed = avg_spawn_speed_ + rand(spawn_speed_var_);
		p.acc = avg_acc_ + rand(acc_var_);
		p.deacc = avg_deacc_ + rand(deacc_var_);
		p.scale = avg_scale_ + rand(scale_var_);
		p.motion_randomization = motion_rand_;
		particles_.push_back(p);
	}
}

void ParticleSystem::update(double dt) {
	if(!enabled)
		return;
	//Spawn new particles:
	float new_particles = regen_*dt+particle_rest_;
	int num_new = (int)new_particles;
	particle_rest_ = new_particles - num_new;
	spawn_particles(num_new);

	//Update particles:
	std::list<particle_t>::iterator it = particles_.begin();
	int kill_count=0;
	while(it != particles_.end()) {
		if(!it->update(dt)) {
			particles_.erase(it++); //Erase and move iterator to next
			++kill_count;
		} else {
			++it; //move to next
		}
	}
	if(particles_.size() > MAX_NUM_PARTICLES) {
		printf("Warning! There are %lu particles, but only %d will render (MAX_NUM_PARTICLES)\n", particles_.size(), MAX_NUM_PARTICLES);
	}
}

void ParticleSystem::render(double dt, Renderer * renderer) {
	if(!enabled)
		return;

	//Update vertices:
	std::list<particle_t>::iterator it = particles_.begin();
	int count=0;
	while(it != particles_.end()) {
		it->update_vertices(vertices_+count*NUM_SIDES*4);
		++count;
		++it;
		if(count>=MAX_NUM_PARTICLES)
			break;
	}

	//cube_->set_position(position_+spawn_area_/2.f);
	//cube_->render(dt, renderer);


	glUseProgram(renderer->shaders[shader_].program);
	glUseProgram(renderer->shaders[Renderer::PARTICLES_SHADER].program);
	glActiveTexture(GL_TEXTURE0);
	texture_->bind();

	glBindBuffer(GL_ARRAY_BUFFER, vb_);
	Renderer::checkForGLErrors("ParticleSystem::render() - bind buffer");
	
	//Upload new vertex data
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t)*count*NUM_SIDES*4, vertices_);
	Renderer::checkForGLErrors("ParticleSystem::render() - Upload new data");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)+sizeof(glm::vec2)));

	//Upload matrix (if parent have changed it)
	renderer->upload_model_matrices(false);
	//Disable depth mask to get alpha blending right
	glDepthMask(GL_FALSE);

	glDrawElements(GL_TRIANGLES, count*6*NUM_SIDES, GL_UNSIGNED_INT, 0);
	Renderer::checkForGLErrors("ParticleSystem::render() - draw");

	//And turn depth mask on again
	glDepthMask(GL_TRUE);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
		
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	texture_->unbind();

	glUseProgram(0);

}

float ParticleSystem::rand(float var, bool d) {
	if(d)
		return (var*2.f)*frand()-var;
	else
		return var*frand();
}
glm::vec3 ParticleSystem::rand(glm::vec3 var, bool d) {
	return glm::vec3(rand(var.x,d), rand(var.y,d), rand(var.z,d));
}

const glm::mat4 ParticleSystem::translation_matrix() const {
	return glm::mat4(1.f); //Identity matrix
}

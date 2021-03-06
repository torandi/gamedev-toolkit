#include "world.h"
#include "light.h"
#include "renderer.h"
#include "render_object.h"

#include "terrain.h"
#include "particle_system.h"

#include <assimp/aiPostProcess.h>
#include <cstdio>
#include <glm/glm.hpp>


Light * lights_lights[NUM_LIGHTS]; //The actual lights
RenderObject * lights_ro[NUM_LIGHTS];
MoveGroup lights[NUM_LIGHTS];
Terrain * t;

ParticleSystem * particles;
ParticleSystem * underwater;

const glm::vec3 night_light = glm::vec3(0.0, 0.15, 0.15);
const glm::vec3 morning_light = glm::vec3(0.6, 0.3, 0.3);
const glm::vec3 day_light = glm::vec3(0.6, 0.6, 0.6);
const glm::vec3 evening_light = glm::vec3(0.5, 0.2, 0.2);

const float high_night = 2;
const float high_morning = 8;
const float high_day = 14;
const float high_evening = 21;

const float time_per_hour=5.f;

float time_of_day = 8.0; //0->24;


void create_world(Renderer * renderer) {

texture_pack_t * terrain_textures;

/*	
ParticleSystem::ParticleSystem(
	glm::vec3 position, glm::vec3 spawn_area_size, float regeneration, float avg_ttl, float ttl_var,
	avg_spawn_speed, spawn_speed_var, avg_acc, acc_var, avg_deacc, deacc_var,
	glm::vec3 spawn_direction, glm::vec3 direction_var, float avg_scale, float scale_var,
	Renderer::shader_program_t shader, std::string texture, glm::vec4 color1, glm::vec4 color2, motion rand
)*/

	particles = new ParticleSystem(glm::vec3(137, 417, 231), glm::vec3(400, 1, 400), 1000, 50, 40, 
		5.f, 3.f, 2.0, 0.6, 0.0, 0.0, 
		glm::vec3(0, -1, 0), glm::vec3(1, 0.5, 1), 1.0, 0.2, 
		Renderer::PARTICLES_SHADER, "textures/particle.png", glm::vec4(1,1,1,0.9), glm::vec4(1, 1, 1,0.8), glm::vec3(5, 1, 5)
		);

	particles->spawn_particles(50000);
	//Run a couple of cycles:
	for(int i=0;i<10;++i) {
		particles->update(1.f);
	}

	std::string tf[] = {"dirt", "sand","grass","mountain", "snow"};
	std::vector<std::string> texture_files(tf, tf+5);
	terrain_textures = Terrain::generate_texture_pack("valley",texture_files);

	Terrain::init_terrain(renderer);
	Renderer::checkForGLErrors("init Terrain");

	//renderer->camera.set_position(glm::vec3(0.0, -7.5, 0.0));
	renderer->camera.set_position(glm::vec3(-2.75, -1.62, 28));
	renderer->camera.relative_rotate(glm::vec3(0, 1, 0), 180);

	//Skybox
	renderer->load_skybox("skybox");

	Texture * water = new Texture("valley/water.dds");

	water->bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	water->unbind();
	Renderer::checkForGLErrors("water params");

	t = new Terrain ("valley",1.f, 200.f, 0.3f, terrain_textures, water, glm::vec2(0,0), glm::vec2(0, 0));
	Renderer::checkForGLErrors("Terrain load");
	t->start_height = 50.f;
	t->relative_move(glm::vec3(0,-200.f,0));

	t->wave1 = glm::vec2(0.1, 0)*0.01f;
	t->wave2 = glm::vec2(0.05, 0.3)*0.01f;

	renderer->render_objects.push_back(t);

	/*underwater = new ParticleSystem(glm::vec3(-t->width()/2.f, -t->water_level(), -t->height()/2.f), glm::vec3(t->width(), t->water_level()+5.f, t->height()), 500, 20, 5, 
		0.25f, 0.20f, 0.1, 0.05, 0.0, 0.0, 
		glm::vec3(0, -1, 0), glm::vec3(1,1.5, 1), 1.0, 0.2, 
		Renderer::PARTICLES_SHADER, "textures/particle.png", glm::vec4(0.5,0.7,1,0.6), glm::vec4(0.5, 0.5, 1,0.6), glm::vec3(0, 0, 0)
		);
	underwater->spawn_particles(50000);
	//Run a couple of cycles:
	for(int i=0;i<10;++i) {
		underwater->update(1.f);
	}*/

	renderer->render_objects.push_back(particles);
	//renderer->render_objects.push_back(underwater);


	//Lights:
#if NUM_LIGHTS > 1
	lights_lights[LIGHT_SOURCE0] = new Light(glm::vec3(0.8, 0.8, 0.8), Light::POINT_LIGHT);
	lights_lights[LIGHT_SOURCE1] = new Light(glm::vec3(0.6, 0.6, 0.6), Light::POINT_LIGHT);
#else
	lights_lights[0] = new Light(glm::vec3(0.8, 0.8, 0.8), Light::POINT_LIGHT);
	lights_lights[0]->set_half_light_distance(10.f);
#endif

	for(int i=0; i < NUM_LIGHTS; ++i) {
		lights_ro[i] = new RenderObject("models/cube.obj", Renderer::NORMAL_SHADER);	
		lights_lights[i]->set_id_in_render_object(lights_ro[i], i, true);
		lights_ro[i]->scale*=0.25f;
		lights[i].add_object(lights_lights[i]);
		lights[i].add_object(lights_ro[i]);
		renderer->render_objects.push_back(lights_ro[i]);
		renderer->lights.push_back(lights_lights[i]);
		lights[i].set_position(glm::vec3(5.0, -9.0, 5.0));
	}
	
	//load models:
/*
	//objects.back().position+=glm::vec3(0.0, 0.0, 0.f);
	renderer->render_objects.push_back(new RenderObject("models/nintendo.obj", Renderer::NORMAL_SHADER));//, true, aiProcess_FixInfacingNormals));
	renderer->render_objects.back()->set_position(glm::vec3(-1.0,-7.0,0.0));


	renderer->render_objects.push_back(new RenderObject("models/sonic.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(5.0,0.0,0.0));
	renderer->render_objects.back()->scale*=5.f;

	renderer->render_objects.push_back(new RenderObject("models/fleur.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(0.0,0.0,12.0));
	renderer->render_objects.back()->scale*=5.f;
	renderer->render_objects.push_back(new RenderObject("models/wall.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(-5.0,0.0,5.0));
	renderer->render_objects.back()->scale*=5.f;
	renderer->render_objects.push_back(new RenderObject("models/cube_textured.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(0.0,0.0,2.0));

	RenderObject * m = new RenderObject("models/mario_obj.obj", Renderer::NORMAL_SHADER);

	mario.add_object(m);
	m->absolute_move(glm::vec3(0.0, 2.0, 0.0));
	renderer->render_objects.push_back(m);
	renderer->render_objects.back()->scale*=3.0f;
*/
	lights[LIGHT_SOURCE0].set_position(glm::vec3(-1.0, 10.0, -2.0));


	
}

void update_world(double dt, Renderer * renderer) {
	/*if(renderer->camera.position().y < (t->matrix()*glm::vec4(0.0, t->water_level(), 0.0, 1.f)).y)
		underwater->enabled = true;
	else
		underwater->enabled = false;*/

	particles->update(dt);
	//underwater->update(dt);

	time_of_day+=dt/time_per_hour;
	time_of_day = fmod(time_of_day, 24.f);
	
	const glm::vec3 * v1, *v2;
	float t1, t2;

	//printf("(%f, %f, %f), (%f, %f, %f)\n", renderer->camera.position().x, renderer->camera.position().y, renderer->camera.position().z, renderer->camera.look_at().x, renderer->camera.look_at().y, renderer->camera.look_at().z);

	if(time_of_day > high_evening || time_of_day < high_night) {
		v1 = &evening_light;
		t1 = high_evening;
	} else if(time_of_day > high_day)  {
		v1 = &day_light;
		t1 = high_day;
	} else if(time_of_day > high_morning) {
		v1 = &morning_light;
		t1 = high_morning;
	} else {
		v1 = &night_light;
		t1 = high_night;
	}

	if(time_of_day <= high_night || time_of_day > high_evening) {
		v2 = &night_light;
		t2 = high_night;
	} else if(time_of_day <= high_morning) {
		v2 = &morning_light;
		t2 = high_morning;
	} else if(time_of_day <= high_day) {
		v2 = &day_light;
		t2 = high_day;
	} else {
		v2 = &evening_light;
		t2 = high_evening;
	}

	//Fade ambient light:
	float total = t2-t1;
	float pos = time_of_day-t1;
	if(t2 == high_night) {
		total=24.f-t1+t2;
		if(time_of_day > t1)
			pos = time_of_day - t1;
		else
			pos = time_of_day + (24.f-t1);
	}
	renderer->ambient_intensity = glm::mix(*v1, *v2, pos/total);
}

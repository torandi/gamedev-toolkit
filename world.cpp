#include "world.h"
#include "light.h"
#include "renderer.h"
#include "render_object.h"

#include <assimp/aiPostProcess.h>
#include <cstdio>


Light * lights_lights[NUM_LIGHTS]; //The actual lights
RenderObject * lights_ro[NUM_LIGHTS];
MoveGroup lights[NUM_LIGHTS];

MoveGroup mario;

const glm::vec3 night_light = glm::vec3(0.0, 0.15, 0.15);
const glm::vec3 morning_light = glm::vec3(0.6, 0.3, 0.3);
const glm::vec3 day_light = glm::vec3(0.8, 0.6, 0.6);
const glm::vec3 evening_light = glm::vec3(0.5, 0.2, 0.2);

const float high_night = 2;
const float high_morning = 8;
const float high_day = 14;
const float high_evening = 21;

const float time_per_hour=10.f;

float time_of_day = 0.0; //0->24;

void create_world(Renderer * renderer) {
	//Skybox
	renderer->load_skybox("skybox");

	//Lights:
#if NUM_LIGHTS > 1
	lights_lights[LIGHT_SOURCE0] = new Light(glm::vec3(0.9, 0.6, 0.2), Light::POINT_LIGHT);
	lights_lights[LIGHT_SOURCE1] = new Light(glm::vec3(0.9, 0.6, 0.2), Light::POINT_LIGHT);
#else
	lights_lights[0] = new Light(glm::vec3(0.8, 0.8, 0.8), Light::POINT_LIGHT);
#endif

	for(int i=0; i < NUM_LIGHTS; ++i) {
		lights_ro[i] = new RenderObject("models/cube.obj", Renderer::LIGHT_SOURCE_SHADER);	
		lights_lights[i]->set_id_in_render_object(lights_ro[i], i, true);
		lights_ro[i]->scale*=0.25f;
		lights[i].add_object(lights_lights[i]);
		lights[i].add_object(lights_ro[i]);
		renderer->render_objects.push_back(lights_ro[i]);
		renderer->lights.push_back(lights_lights[i]);
		lights[i].set_position(glm::vec3(1.0, 1.0, 1.0));
	}

	//load models:

	renderer->render_objects.push_back(new RenderObject("models/earth.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(0.0,3.0,0.0));

	RenderObject * m = new RenderObject("models/mario_obj.obj", Renderer::NORMAL_SHADER);

	mario.add_object(m);

	renderer->render_objects.push_back(m);
	renderer->render_objects.back()->scale*=3.0f;

	//objects.back().position+=glm::vec3(0.0, 0.0, 0.f);
	renderer->render_objects.push_back(new RenderObject("models/nintendo.obj", Renderer::NORMAL_SHADER));//, true, aiProcess_FixInfacingNormals));
	renderer->render_objects.back()->absolute_move(glm::vec3(-2.0,0.0,0.0));


}

void update_world(double dt, Renderer * renderer) {
	//printf("Light pos: (%f, %f, %f)\n", lights_ro[LIGHT_SOURCE1]->position().x, lights_ro[LIGHT_SOURCE1]->position().y, lights_ro[LIGHT_SOURCE1]->position().z);
	mario.absolute_rotate(glm::vec3(1.0, 1.0, 1.0), dt*20.f);

	time_of_day+=dt/time_per_hour;
	time_of_day = fmod(time_of_day, 24.f);
	
	const glm::vec3 * v1, *v2;
	float t1, t2;

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

	//Fade amient light:
	printf("Current time: %.2f checkpoint: %f->%f\n", time_of_day, t1, t2);
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

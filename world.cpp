#include "world.h"
#include "light.h"
#include "renderer.h"
#include "render_object.h"

#include <cstdio>


Light * lights_lights[NUM_LIGHTS]; //The actual lights
RenderObject * lights_ro[NUM_LIGHTS];
MoveGroup lights[NUM_LIGHTS];

MoveGroup mario;

void create_world(Renderer * renderer) {
	//Lights:
	lights_lights[LIGHT_SOURCE0] = new Light(glm::vec3(0.8, 0.8, 0.8), Light::DIRECTIONAL_LIGHT);
	//lights_lights[LIGHT_SOURCE1] = new Light(glm::vec3(0.5, 0.5, 0.5), Light::DIRECTIONAL_LIGHT);

	for(int i=0; i < NUM_LIGHTS; ++i) {
		lights_ro[i] = new RenderObject("models/cube.obj", Renderer::NORMAL_SHADER);	
		lights_ro[i]->scale*=0.25f;
		lights[i].add_object(lights_lights[i]);
		lights[i].add_object(lights_ro[i]);
		renderer->render_objects.push_back(lights_ro[i]);
		renderer->lights.push_back(lights_lights[i]);
	}

	lights[LIGHT_SOURCE0].set_position(glm::vec3(1.0, 1.0, 1.0));
	//lights[LIGHT_SOURCE1].set_position(glm::vec3(5.0, -5.0, 5.0));

	//load models:

	renderer->render_objects.push_back(new RenderObject("models/ninja.b3d", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(0.0,0.0,5.0));
	renderer->render_objects.push_back(new RenderObject("models/earth.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(0.0,3.0,0.0));

	RenderObject * m = new RenderObject("models/mario_obj.obj", Renderer::NORMAL_SHADER);

	mario.add_object(m);

	renderer->render_objects.push_back(m);
	renderer->render_objects.back()->scale*=3.0f;

	//objects.back().position+=glm::vec3(0.0, 0.0, 0.f);
/*	renderer->render_objects.push_back(new RenderObject("models/nintendo.obj", Renderer::NORMAL_SHADER));
	renderer->render_objects.back()->absolute_move(glm::vec3(-2.0,0.0,0.0));*/
}

void update_world(double dt, Renderer * renderer) {
	//printf("Light pos: (%f, %f, %f)\n", lights_ro[LIGHT_SOURCE1]->position().x, lights_ro[LIGHT_SOURCE1]->position().y, lights_ro[LIGHT_SOURCE1]->position().z);
	mario.absolute_rotate(glm::vec3(1.0, 1.0, 1.0), dt*20.f);
}

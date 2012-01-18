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

void create_world(Renderer * renderer) {
	//Lights:
#if NUM_LIGHTS > 1
	lights_lights[LIGHT_SOURCE0] = new Light(glm::vec3(0.2, 0.2, 0.2), Light::POINT_LIGHT);
	lights_lights[LIGHT_SOURCE1] = new Light(glm::vec3(0.0, 0.0, 0.8), Light::POINT_LIGHT);
	lights_lights[LIGHT_SOURCE2] = new Light(glm::vec3(1.0, 1.0, 0.0), Light::POINT_LIGHT);
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
}

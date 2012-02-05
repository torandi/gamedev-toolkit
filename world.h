#ifndef WORLD_H
#define WORLD_H
	#include "renderer.h"
	#include "move_group.h"
	#include "render_object.h"
	#include "light.h"
	#include "particle_system.h"

	#define NUM_LIGHTS 1

	enum {
		LIGHT_SOURCE0,
		LIGHT_SOURCE1,
	};

	extern Light *lights_lights[NUM_LIGHTS]; //The actual lights
	extern RenderObject* lights_ro[NUM_LIGHTS];
	extern MoveGroup lights[NUM_LIGHTS];
	extern ParticleSystem * particles;


	void create_world(Renderer * renderer);
	void update_world(double dt, Renderer * renderer);
#endif

#include <cstdio>
#include <cmath>
#include <SDL/SDL.h>
#include "renderer.h"
#include "input.h"
#include "util.h"
#include "world.h"

#define LIGHT_ROTATION_SPEED M_PI/8.f

#define MOVE_SPEED 100.f
#define ROTATION_SPEED 50.f

float cam_rot = 0;
float lit_rot = 0;
//float cam_center_distance = 5.0f;
float lit_center_distance = 2.0f;

int move_light = -1;
bool last_a_btn_status = false;

void logic(double dt, Renderer * renderer) {
	if(button_down(0) && !last_a_btn_status) {
		last_a_btn_status = true;
		++move_light;
		if(move_light >= NUM_LIGHTS)
			move_light = -1;
		printf("Toggle move light to: %d\n", move_light);
	} else if(!button_down(0)) {
		last_a_btn_status = false;
	}
	
	float x = 0;
	float y = 0;
	float z = 0;
	float rx = 0;
	float ry = 0; 
	float rz = 0;

	if(joy != NULL) {
		x = -normalized_axis_value(0)*MOVE_SPEED*dt - get_hat_right_left(0)*MOVE_SPEED*dt;;
		y = get_hat_up_down(0)*MOVE_SPEED*dt;
		if(button_down(5))
			y = normalized_axis_value(1)*MOVE_SPEED*dt;
		else
			z = -normalized_axis_value(1)*MOVE_SPEED*dt;
		rx = -normalized_axis_value(4)*ROTATION_SPEED*dt;
		ry = -(normalized_trigger_value(5) - normalized_trigger_value(2))*2.0*ROTATION_SPEED*dt;
		rz = normalized_axis_value(3)*ROTATION_SPEED*dt;
	} else {
		z = (keys[SDLK_COMMA]-keys[SDLK_o])*MOVE_SPEED*dt;
		x = (keys[SDLK_a]-keys[SDLK_e])*MOVE_SPEED*dt;
		y = (keys[SDLK_UP]-keys[SDLK_DOWN])*MOVE_SPEED*dt;

	}
	if(move_light != -1) {
		y = -normalized_axis_value(4)*MOVE_SPEED*dt;
		z = -normalized_axis_value(1)*MOVE_SPEED*dt;

		lights[move_light].absolute_move(glm::vec3(x, y, z));
	} else {

		if(fabs(x) > 0 || fabs(y) > 0 || fabs(z) > 0) 
			renderer->camera.relative_move(glm::vec3(x, y, z));

		if(fabs(rx) > 0) 
			renderer->camera.relative_rotate(glm::vec3(1.f, 0.f, 0.f), ROTATION_SPEED*dt*rx);
		if(fabs(rz) > 0) 
			renderer->camera.relative_rotate(glm::vec3(0.f, 0.f, 1.f), rz);
		if(fabs(ry) > 0) 
			renderer->camera.relative_rotate(glm::vec3(0.f, 1.f, 0.f), ry);
	}
}

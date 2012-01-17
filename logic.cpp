#include <cstdio>
#include <cmath>
#include <SDL/SDL.h>
#include "renderer.h"
#include "input.h"
#include "util.h"
#include "world.h"

#define LIGHT_ROTATION_SPEED M_PI/8.f

#define MOVE_SPEED 5.f
#define ROTATION_SPEED 40.f

float cam_rot = 0;
float lit_rot = 0;
//float cam_center_distance = 5.0f;
float lit_center_distance = 2.0f;

int move_light = -1;
bool last_a_btn_status = false;

void logic(double dt, Renderer * renderer) {
	//Rotate light
/*	lit_rot+= LIGHT_ROTATION_SPEED*dt;
	lit_rot = fmod(lit_rot, M_PI*2);	
	lights.back().position  = glm::vec4(cos(lit_rot+M_PI), 0.0, sin(lit_rot+M_PI), 1.0)*lit_center_distance;*/

	//Move camera:
	/*
	camera_pos.x+= 
	camera_pos.z-= normalized_axis_value(1)*MOVE_SPEED*dt;
	if(normalized_axis_value(0) != 0.f)
		printf("Axis X: %f\n", normalized_axis_value(0));
	if(normalized_axis_value(1) != 0.f)
		printf("Axis Z: %f\n", normalized_axis_value(1));
	*/
	if(button_down(0) && !last_a_btn_status) {
		last_a_btn_status = true;
		++move_light;
		if(move_light >= NUM_LIGHTS)
			move_light = -1;
		printf("Toggle move light to: %d\n", move_light);
	} else if(!button_down(0)) {
		last_a_btn_status = false;
	}
	
	float x = -normalized_axis_value(0)*MOVE_SPEED*dt - get_hat_right_left(0)*MOVE_SPEED*dt;;
	float y = get_hat_up_down(0)*MOVE_SPEED*dt;
	float z;
	if(button_down(5))
		y = -normalized_axis_value(1)*MOVE_SPEED*dt;
	else
		z = -normalized_axis_value(1)*MOVE_SPEED*dt;
	float rx = -normalized_axis_value(4)*ROTATION_SPEED*dt;
	float ry = (normalized_trigger_value(2)-normalized_trigger_value(5))*ROTATION_SPEED*dt;
	float rz = normalized_axis_value(3)*ROTATION_SPEED*dt;

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
	//if(fabs(rx) > 0) 
	//if(fabs(rx) > 0) 

	//camera_pos.x+= normalized_axis_value(0)*MOVE_SPEED;
	//camera_pos.y+= normalized_axis_value(1)*MOVE_SPEED;
}

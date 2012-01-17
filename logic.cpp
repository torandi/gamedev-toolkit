#include <cstdio>
#include <cmath>
#include <SDL/SDL.h>
#include "render.h"
#include "input.h"
#include "util.h"

#define LIGHT_ROTATION_SPEED M_PI/8.f

#define MOVE_SPEED 1.f
#define ROTATION_SPEED 10.f

float cam_rot = 0;
float lit_rot = 0;
//float cam_center_distance = 5.0f;
float lit_center_distance = 2.0f;


void logic(double dt) {
	//Rotate light
	lit_rot+= LIGHT_ROTATION_SPEED*dt;
	lit_rot = fmod(lit_rot, M_PI*2);	
	lights.back().position  = glm::vec4(cos(lit_rot+M_PI), 0.0, sin(lit_rot+M_PI), 1.0)*lit_center_distance;

	//Move camera:
	/*
	camera_pos.x+= 
	camera_pos.z-= normalized_axis_value(1)*MOVE_SPEED*dt;
	if(normalized_axis_value(0) != 0.f)
		printf("Axis X: %f\n", normalized_axis_value(0));
	if(normalized_axis_value(1) != 0.f)
		printf("Axis Z: %f\n", normalized_axis_value(1));
	*/
	float x = -normalized_axis_value(0)*MOVE_SPEED*dt;
	float y = (normalized_trigger_value(2)-normalized_trigger_value(5))*MOVE_SPEED*dt;
	float z = -normalized_axis_value(1)*MOVE_SPEED*dt;
	float rx = normalized_axis_value(4)*ROTATION_SPEED*dt;
	float ry = normalized_axis_value(3)*ROTATION_SPEED*dt;
	//float rz = normalized_axis_value(4)*MOVE_SPEED*dt;
	
	if(fabs(x) > 0 || fabs(y) > 0 || fabs(z) > 0) 
		camera.relative_move(glm::vec3(x, y, z));

	if(fabs(rx) > 0) 
		camera.relative_rotate(glm::vec3(1.f, 0.f, 0.f), rx);
	if(fabs(ry) > 0) 
		camera.relative_rotate(glm::vec3(0.f, 1.f, 0.f), ry);
	/*if(fabs(rz) > 0) 
		camera.relative_rotate(glm::vec3(0.f, 0.f, 1.f), ROTATION_SPEED*dt*rz);*/
	//if(fabs(rx) > 0) 
	//if(fabs(rx) > 0) 

	//camera_pos.x+= normalized_axis_value(0)*MOVE_SPEED;
	//camera_pos.y+= normalized_axis_value(1)*MOVE_SPEED;
}

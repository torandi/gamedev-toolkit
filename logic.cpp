#include <cstdio>
#include <cmath>
#include "render.h"

#define ROTATION_SPEED M_PI/8.f

float cam_rot = 0;
float lit_rot = 0;
float cam_center_distance = 10.0f;
float lit_center_distance = 10.0f;

static float deg_to_rad(float fAngDeg)
{
	const float fDegToRad = 3.14159f * 2.0f / 360.0f;
	return fAngDeg * fDegToRad;
}

static float rad_to_deg(float fAngRad)
{
	const float fDegToRad = 3.14159f * 2.0f / 360.0f;
	return fAngRad / fDegToRad;
}

void logic(double dt) {
	cam_rot+= ROTATION_SPEED*dt;
	cam_rot = fmod(cam_rot, M_PI*2);	
	lit_rot+= 2*ROTATION_SPEED*dt;
	lit_rot = fmod(lit_rot, M_PI*2);	
	camera_pos = glm::vec3(cos(cam_rot), 0.0, sin(cam_rot))*cam_center_distance;
	light_pos= glm::vec4(cos(lit_rot+M_PI), 2.0, sin(lit_rot+M_PI), 1.0)*lit_center_distance;
}

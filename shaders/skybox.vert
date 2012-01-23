#version 330
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec3 in_texcoord;

out vec3 texcoord;

void main() {
	gl_Position = projectionViewMatrix * in_position;
	texcoord = in_texcoord;
	texcoord.x-=0.5;
	texcoord.y-=0.5;
	if(in_texcoord.z==0.f) {
		texcoord.z = 0.5;
		texcoord.y*=-1.0;
	} else if(in_texcoord.z==1.f) {
		texcoord.z = -0.5;
		texcoord.xy*=-1.0;
	} else if(in_texcoord.z == 2.f) {
		texcoord.z=texcoord.x;
		texcoord.x = 0.5f;
		texcoord.y*=-1.0f;
	} else if(in_texcoord.z == 3.f) {
		texcoord.z = texcoord.x*-1.0;
		texcoord.x = -0.5;
		texcoord.y*=-1.0f;
	} else if(in_texcoord.z == 4.f) {
		texcoord.z = texcoord.y*-1.f;
		texcoord.y = 0.5;
	} else if(in_texcoord.z == 5.f) {
		texcoord.z = texcoord.y;
		texcoord.y = -0.5;
	}

}


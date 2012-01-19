#version 330
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;

out vec2 texcoord;

void main() {
	gl_Position = projectionViewMatrix * in_position;
	texcoord = in_texcoord;
}

